var sel = document.getElementById("show-map");
sel.addEventListener("click", function () {
  initMap();
});

var time=document.getElementById("selectHour");
time.addEventListener("change", function () {
  $("#show-map").show();
});
let infoWindow;

function initMap() {
  infoWindow = new google.maps.InfoWindow();
  var latitude = $("#latitude").html();
  var longitude = $("#longitude").html();
  console.log(latitude, longitude);
  const mark = { lat: parseFloat(latitude), lng: parseFloat(longitude) };

  const map = new google.maps.Map(document.getElementById("map"), {
    center: mark,
    zoom: 17,
  });

  const marker = new google.maps.Marker({
    position: mark,
    map: map,
  });

  const locationButton = document.createElement("button");
  locationButton.textContent = "Pan to Current Location";
  locationButton.classList.add("custom-map-control-button");
  map.controls[google.maps.ControlPosition.TOP_CENTER].push(locationButton);
  
  locationButton.addEventListener("click", () => {
    if (navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(
        (position) => {
          const pos = {
            lat: position.coords.latitude,
            lng: position.coords.longitude,
          };

          infoWindow.setPosition(pos);
          infoWindow.setContent("Location found.");
          infoWindow.open(map);
          map.setCenter(pos);
        },
        () => {
          handleLocationError(true, infoWindow, map.getCenter());
        }
      );
    } else {
      // Browser doesn't support Geolocation
      handleLocationError(false, infoWindow, map.getCenter());
    }
  });
}

window.initMap = initMap;