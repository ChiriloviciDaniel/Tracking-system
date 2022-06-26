#include <SoftwareSerial.h>
#include <MicroNMEA.h>

//sim800l(TXpin,RXpin)
SoftwareSerial sim800l(3, 2);

//gps8M(TXpin,RXpin);
SoftwareSerial gps8M(8, 7);

char buffer[85];
MicroNMEA nmea(buffer, sizeof(buffer));
int flagSMS, flagERROR;

long timeout = 60000;      //60000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  gps8M.begin(9600);
  Serial.println("====Start====");
  sim800l.print("AT");
}

void loop() {

  while (sim800l.available()) {
    Serial.println(sim800l.readString());
  }
  while (Serial.available())  {
    sim800l.println(Serial.readString());
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > timeout) {
    previousMillis = currentMillis;
    getLocation();
  }
}

void reset() {
  delay(1000);
  //AT+CFUN=0  FLIGHT MOD RETURN - NOT READY
  sendATcommand("AT+CFUN=0", "NOT READY", 6000);
  delay(1000);
  //AT+CFUN=1  FLIGHT MOD OFF RETURN - READY
  sendATcommand("AT+CFUN=1", "READY", 6000);
}

void getLocation() {
  gps8M.listen();
  float latGPS = 0.00, lonGPS = 0.00;
  int gpsHH, gpsMM, gpsSS, gpsMm, gpsDD;
  int numSatelites = 0, gpsYY = 0;
  String gpsTime = "";
  String gpsData = "";
  while (latGPS == 0 || latGPS == 999000000.00) {
    if (gps8M.available()) {
      char c = gps8M.read();
      if (nmea.process(c)) {
        latGPS = nmea.getLatitude();
        lonGPS = nmea.getLongitude();
        gpsHH = nmea.getHour();
        gpsMM = nmea.getMinute();
        gpsSS = nmea.getSecond();
        gpsTime = String(nmea.getHour()) + ":" + String(nmea.getMinute()) + ":" + String(nmea.getSecond());

        gpsYY = nmea.getYear();
        gpsMm = nmea.getMonth();
        gpsDD = nmea.getDay();
        gpsData = String(nmea.getYear()) + ":" + String(nmea.getMonth()) + ":" + String(nmea.getDay());
        String url = "asia-southeast1.firebasedatabase.app/" + gpsData + "/" + gpsTime;
        Serial.println(gpsYY);

        if (lonGPS != 0 && lonGPS != 999000000.00) {
          delay(5000);
          Serial.println(latGPS);
          Serial.println(lonGPS);
          delay(10000);
          postDB(latGPS, lonGPS, url);
        }
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >  120000) { //2 min timeout   //60 000ms=1 min
          previousMillis = currentMillis;
          Serial.println("timeout");
          break;
        }
      }
    }
  }
}

void postDB(float latitudeGPS, float longitudeGPS, String url) {

  //initialize flags
  flagSMS = 0;
  //for each error in sendDB() this flag=flag+1
  flagERROR = 0;

  sim800l.listen();
  //Connection type: GPRS - bearer profile 1
  Serial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendATcommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", 9000);
  delay(1000);

  //set the APN settings for Orange network provider.
  Serial.println("AT+SAPBR=3,1,\"APN\",\"net\"");
  sendATcommand("AT+SAPBR=3,1,\"APN\",\"net\"", "OK", 6000);
  delay(1000);

  //enable the GPRS - enable bearer 1
  Serial.println("AT+SAPBR=1,1");
  sendATcommand("AT+SAPBR=1,1", "OK", 6000);
  delay(1000);

  //Init HTTP service
  Serial.println("AT+HTTPINIT");
  sendATcommand("AT+HTTPINIT", "OK", 6000);
  delay(1000);

  //comm
  Serial.println("AT+HTTPSSL=1");
  sendATcommand("AT+HTTPSSL=1", "OK", 6000);
  delay(1000);

  //get ip
  Serial.println("AT+HTTPPARA=\"CID\",1");
  sendATcommand("AT+HTTPPARA=\"CID\",1", "OK", 6000);
  delay(1000);

  //Set the http URL
  sim800l.print("AT+HTTPPARA=\"URL\",\"https://arduino-test-2db0d-default-rtdb.");
  sim800l.print(url);
  sendATcommand(".json?H0GQITACvuqlD7LYIbEmTEHmTUzdF2cflD7UvIVT\"", "OK", 6000);
  delay(1000);

  //prepare date
  Serial.println("AT+HTTPDATA=100,10000");
  sendATcommand("AT+HTTPDATA=100,10000", "DOWNLOAD", 6000);

  //test to error and send txt msj
  //sendATcommand("\"{TEST1\":\"test}\"", "OK", 9000);
  //"{ \"name17\": \"-test432\" }"
  sendATcommand("{\"latitude\":\"" + String(latitudeGPS / 1000000, 6) + "\",\"longitude\":\"" + String(longitudeGPS / 1000000, 6) + "\"}", "OK", 9000);
  //test- send a simple data to db
  //sendATcommand("{\"test\":\"test2\"}", "OK", 9000);
  // delay(1000);

  //Set up the HTTP action 0-get|1-post|...
  Serial.println("AT+HTTPACTION=1");
  sendATcommand("AT+HTTPACTION=1", "1,200", 6000);
  delay(1000);

  //Read status
  Serial.println("AT+HTTPREAD");
  sendATcommand("AT+HTTPREAD", "name", 6000);
  delay(1000);

  //Terminate the HTTP service
  Serial.println("AT+HTTPTERM");
  sendATcommand("AT+HTTPTERM", "OK", 6000);
  delay(1000);

  //shuts down the GPRS connection. This returns "SHUT OK".
  Serial.println("AT+SAPBR=0,1");
  sendATcommand("AT+SAPBR=0,1", "OK", 6000);
  delay(1000);

  if (flagSMS > 0) {
    sendSMS(latitudeGPS, longitudeGPS);
  }
  if (flagERROR > 0) {
    reset();
  }
}

void sendSMS(float latitudeGPS, float longitudeGPS) {

  sendATcommand("AT+CMGF=1", "OK", 6000);
  delay(1000);
  sendATcommand("AT+CMGS=\"+40785901105\"", ">", 6000);
  delay(1000);
  sim800l.print("Latitude-" + (String)latitudeGPS + " ,Longitude-" + (String)longitudeGPS);

  delay(3000);
  sim800l.write(26);
  flagSMS = 0;
}

void sendATcommand(String ATcomm, String expected_answer, unsigned int timeout)
{
  int reply = 0;
  String resp;
  unsigned long previous;
  sim800l.listen();
  //Clean the input buffer
  while ( sim800l.available() > 0) sim800l.read();
  sim800l.println(ATcomm);

  previous = millis();
  do {
    //if data in UART INPUT BUFFER, reads it
    if (sim800l.available() != 0) {
      resp = sim800l.readString();
      //checks if the (response == expected_answer)
      if (resp.indexOf(expected_answer) > 0) {
        reply += 1;
      }
      else if (resp.indexOf("400") > 0) {
        flagSMS += 1;
      }
      else if (resp.indexOf("ERROR") > 0) {
        flagERROR += 1;
      }
    }
  } while ((reply == 0) && ((millis() - previous) < timeout));

  Serial.println(resp);
}

void displayData(float latitude, float longitude, int numSatelites, String gpsTime) {
  Serial.print("GPS latitude: ");
  Serial.println(latitude / 1000000, 8);
  Serial.print("GPS longitude: ");
  Serial.println(longitude / 1000000, 8);
  Serial.print("GPS satelites: ");
  Serial.println(numSatelites);
  Serial.print("GPS TIME-");
  Serial.println(gpsTime);
}
