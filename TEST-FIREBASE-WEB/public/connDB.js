import { initializeApp } from "https://www.gstatic.com/firebasejs/9.7.0/firebase-app.js";
// TODO: Add SDKs for Firebase products that you want to use

// https://firebase.google.com/docs/web/setup#available-libraries
import { getDatabase, ref, get, child } from "https://www.gstatic.com/firebasejs/9.7.0/firebase-database.js";

// Your web app's Firebase configuration
const year = document.getElementById('selectYear');
year.addEventListener("change", hours);

const hour = document.getElementById('selectHour');
hour.addEventListener("change", coordonate);


// Initialize Firebase
const firebaseConfig = {
  apiKey: "AIzaSyCHW_E4kU2tsO19pjfNrtz0oQ68cBfgQXM",
  authDomain: "arduino-test-2db0d.firebaseapp.com",
  databaseURL: "https://arduino-test-2db0d-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "arduino-test-2db0d",
  storageBucket: "arduino-test-2db0d.appspot.com",
  messagingSenderId: "424942754086",
  appId: "1:424942754086:web:17b6536e4d0fca979f09e7"
}

const app = initializeApp(firebaseConfig);
const dbRef = ref(getDatabase());

get(child(dbRef, "/"))
  .then((snapshot) => {
    var select = document.getElementById("selectYear");
    if (snapshot.exists()) {
      snapshot.forEach(function (child) {

        var hours = child.val();
        var option = document.createElement('option');
        option.text = option.value = child.key;
        select.add(option, 0);

      });
    } else {
      console.log("No data available");
    }
  })
  .catch((error) => {
    console.error(error);
  });

function hours() {
  document.getElementById("selectHour").options.length = 0;
  var data = document.getElementById("selectYear").value;
  //console.log(data);
  get(child(dbRef, "/" + data + "/"))
    .then((snapshot) => {
      var select = document.getElementById("selectHour");
      var option = document.createElement('option');
      option.text = "Select Hour";
      option.hidden = true;
      select.add(option, 0);
      if (snapshot.exists()) {
        snapshot.forEach(function (child) {
          var hours = child.val();
          var option = document.createElement('option');
          option.text = option.value = child.key;
          select.add(option, 0);

        });
      } else {
        console.log("No data available");
      }
    })
    .catch((error) => {
      console.error(error);
    });
  $(".time").show();

}

function coordonate() {

  var data = document.getElementById("selectYear").value;
  var time = document.getElementById("selectHour").value;

  get(child(dbRef, "/" + data + "/" + time + "/"))
    .then((snapshot) => {
      if (snapshot.exists()) {
        snapshot.forEach(function (child) {

          var p1 = document.getElementById("latitude");
          var p2 = document.getElementById("longitude");

          var lat = Object.values(child.val())[0];
          var lon = Object.values(child.val())[1];

          p1.innerHTML = lat;
          p2.innerHTML = lon;

        });
      } else {
        console.log("No data available");
      }
    })
    .catch((error) => {
      console.error(error);
    });

}