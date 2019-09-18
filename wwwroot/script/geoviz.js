/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const SUPPORT_CONTAINER_ID = 'support_cards';
const SUPPORT_CARD_CLASS = 'gv-support';
const SUPPORT_CARD_INNER_CLASS = 'gv-support-inner';

// Focus on a specific support card.
function focusSupportCard(cardId = null) {
  let card = document.getElementById(cardId);
  for (let c of document.getElementsByClassName(SUPPORT_CARD_CLASS)) {
    if (c !== card) {
      c.children[0].style.height = '0%';
      c.style.height = '0%';
    }
  }
  if (cardId !== null) {
    card.children[0].style.height = '100%';
    card.style.height = '100%';
  } else {
    toggleNavigation();
  }
}

// Callback function to add the response text as support card.
function setResponseAsSupportCard(cardId) {
  return function(request) {
    // Add the request content into the content item.
    document.getElementById(cardId).children[0].innerHTML =
      request.responseText;
  };
}

// Add a support card and open it.
function tryAddSupportCard(url, cardId, gainFocus = true) {
  // Always disable the menu.
  toggleNavigation();

  // Check whether the element already exists.
  const element = document.getElementById(cardId);
  if (element === null) {
    // Create a new section element in the container that indicates that the
    // content is loading.
    document.getElementById(SUPPORT_CONTAINER_ID).innerHTML +=
      '<section id="' +
      cardId +
      '" class="aga-panel aga-card ' +
      SUPPORT_CARD_CLASS +
      '" style="height: 0%;">' +
      '<div class=' +
      SUPPORT_CARD_INNER_CLASS +
      '><div class="aga-fill aga-center"><p>Loading content...</p></div></div>' +
      '</section>';

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsSupportCard(cardId));
  }

  if (gainFocus) focusSupportCard(cardId);
}

function initMap() {
  var map = L.map('map').fitWorld();
  // var map = L.map("map").setView([51.448, 5.49], 16);

  L.tileLayer(
    'https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw',
    {
      maxZoom: 18,
      attribution:
        'Map data &copy; <a href="https://www.openstreetmap.org/">OpenStreetMap</a> contributors, ' +
        '<a href="https://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, ' +
        'Imagery © <a href="https://www.mapbox.com/">Mapbox</a>',
      id: 'mapbox.streets'
    }
  ).addTo(map);

  L.control.scale().addTo(map);

  function onLocationFound(e) {
    const radius = e.accuracy / 2;

    L.marker(e.latlng)
      .addTo(map)
      .bindPopup('You are within ' + radius + ' meters from this point')
      .openPopup();

    L.circle(e.latlng, radius).addTo(map);
  }

  function onLocationError(e) {
    // Focus on TU/e.
    map.setView([51.448, 5.49], 16);
  }

  map.on('locationfound', onLocationFound);
  map.on('locationerror', onLocationError);

  map.locate({ setView: true, maxZoom: 16 });

  // L.marker([51.448, 5.49]).addTo(map);

  /*L.circle([51.448, 5.49], {
      color: "red",
      fillColor: "#f03",
      fillOpacity: 0.5,
      radius: 500
    }).addTo(map);*/

  // L.polygon([[51.509, -0.08], [51.503, -0.06], [51.51, -0.047]]).addTo(map);

  // Make sure that the navigation menu and support cards always show on top of
  // the map.
  const mapZIndexMax = Math.max(
    getComputedStyle(document.querySelector('.leaflet-control')).zIndex,
    getComputedStyle(document.querySelector('.leaflet-top')).zIndex
  );
  let stylesheet = document.styleSheets[document.styleSheets.length - 1];
  stylesheet.insertRule('.aga-card { z-index: ' + (mapZIndexMax + 1) + ';}');
  stylesheet.insertRule('.aga-header { z-index: ' + (mapZIndexMax + 2) + ';}');
}
