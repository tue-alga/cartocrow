"use strict";

// Callback function to add the response text as item of the content container.
function setResponseAsContentMapItem(content_item_id) {
    return function (request) {
      // Add the request content into the content item.
      document.getElementById(content_item_id).innerHTML = request.responseText;
  
      var mymap = L.map("mapid").setView([51.448, 5.49], 16);
  
      L.tileLayer(
        "https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw", {
          maxZoom: 18,
          attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/">OpenStreetMap</a> contributors, ' +
            '<a href="https://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, ' +
            'Imagery © <a href="https://www.mapbox.com/">Mapbox</a>',
          id: "mapbox.streets"
        }
      ).addTo(mymap);
  
      /*L.marker([51.5, -0.09]).addTo(mymap);
  
      L.circle([51.508, -0.11], {
        color: "red",
        fillColor: "#f03",
        fillOpacity: 0.5,
        radius: 500
      }).addTo(mymap);
  
      L.polygon([
        [51.509, -0.08],
        [51.503, -0.06],
        [51.51, -0.047]
      ]).addTo(
        mymap
      );*/
    };
  }
  
  // Add a leaflet map section to the section container.
  function tryAddMapSection(
    url,
    content_container_id,
    content_item_id
  ) {
    // Check whether the element already exists.
    let element = document.getElementById(content_item_id);
    if (element === null) {
      // Create a new section element in the container that indicates that the content is loading.
      document.getElementById(content_container_id).innerHTML +=
        '<section id="' + content_item_id + '" class="' + content_item_class + '">' +
        '<div style="display: flex; justify-content: center; align-items: center;"><p>Loading content...</p></div>' +
        '</section>';
  
      // Set focus to the item.
      focusContentItem(content_item_id);
  
      // Get the item content from an external URL.
      ajaxGet(url, setResponseAsContentMapItem(content_item_id));
    } else
    {
       focusContentItem(content_item_id);
    }
  }