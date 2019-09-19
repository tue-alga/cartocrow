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
    // Note that the initial height is set to 0 to hide the card.
    document.getElementById(SUPPORT_CONTAINER_ID).innerHTML +=
      '<section id="' +
      cardId +
      '" class="aga-panel aga-card ' +
      SUPPORT_CARD_CLASS +
      '" style="height: 0;">' +
      '<div class=' +
      SUPPORT_CARD_INNER_CLASS +
      '><div class="aga-fill aga-center"><p>Loading content...</p></div></div>' +
      '</section>';

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsSupportCard(cardId));
  }

  if (gainFocus) focusSupportCard(cardId);
}

// Initialize the map block using Leaflet.
function initMap() {
  var map = L.map('map').fitWorld();

  // Use CartoDB wihtout labels as base layer.
  getTileLayer({ server: tileServer.CARTODB, id: 'light_nolabels' }).addTo(map);

  // Add CartoDB labels as separate pane.
  map.createPane('labels');
  let paneLabels = map.getPane('labels');
  paneLabels.style.zIndex = 650; // On top of markers but below pop-ups
  paneLabels.style.pointerEvents = 'none'; // Disable mouse events on this particular pane.
  getTileLayer({
    server: tileServer.CARTODB,
    id: 'light_only_labels',
    pane: 'labels'
  }).addTo(map);

  //L.gridLayer.debugCoords().addTo(map);

  // TMP TESTING!

  var svgElement = document.createElementNS(
    'http://www.w3.org/2000/svg',
    'svg'
  );
  svgElement.setAttribute('xmlns', 'http://www.w3.org/2000/svg');
  svgElement.setAttribute('width', '250');
  svgElement.setAttribute('viewBox', '0 0 200 85');
  svgElement.setAttribute('version', '1.1');
  //svgElement.setAttribute('viewBox', '0 0 200 200');
  //svgElement.innerHTML = '<rect width="200" height="200"/><rect x="75" y="23" width="50" height="50" style="fill:red"/><rect x="75" y="123" width="50" height="50" style="fill:#0013ff"/>';
  svgElement.innerHTML =
    'Sorry, your browser does not support the svg tag. <defs> <!-- Filter declaration --> <filter id="MyFilter" filterUnits="userSpaceOnUse" x="0" y="0" width="200" height="120" > <!-- offsetBlur --> <feGaussianBlur in="SourceAlpha" stdDeviation="4" result="blur" /> <feOffset in="blur" dx="4" dy="4" result="offsetBlur" />  <!-- litPaint --> <feSpecularLighting in="blur" surfaceScale="5" specularConstant=".75" specularExponent="20" lighting-color="#bbbbbb" result="specOut" > <fePointLight x="-5000" y="-10000" z="20000" /> </feSpecularLighting> <feComposite in="specOut" in2="SourceAlpha" operator="in" result="specOut" /> <feComposite in="SourceGraphic" in2="specOut" operator="arithmetic" k1="0" k2="1" k3="1" k4="0" result="litPaint" />  <!-- merge offsetBlur + litPaint --> <feMerge> <feMergeNode in="offsetBlur" /> <feMergeNode in="litPaint" /> </feMerge> </filter> </defs>  <!-- Graphic elements --> <g filter="url(#MyFilter)"> <path fill="none" stroke="#D90000" stroke-width="10" d="M50,66 c-50,0 -50,-60 0,-60 h100 c50,0 50,60 0,60z" /> <path fill="#D90000" d="M60,56 c-30,0 -30,-40 0,-40 h80 c30,0 30,40 0,40z" /> <g fill="#FFFFFF" stroke="black" font-size="45" font-family="Verdana"> <text x="52" y="52">SVG</text> </g> </g>';
  //var svgElementBounds = [[32, -130], [13, -100]];
  var svgElementBounds = [[51.449, 5.48], [51.447, 5.5]];
  L.svgOverlay(svgElement, svgElementBounds).addTo(map);
  // < TMP TESTING!

  // Add a scale control.
  L.control.scale().addTo(map);

  function onLocationFound(e) {
    // Focus on the user's location.
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

  // Try to locate the user.
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

  // Make sure that the navigation and support cards always show on top of the map.
  const mapZIndexMax = Math.max(
    getComputedStyle(document.querySelector('.leaflet-control')).zIndex,
    getComputedStyle(document.querySelector('.leaflet-top')).zIndex
  );
  let stylesheet = document.styleSheets[document.styleSheets.length - 1];
  stylesheet.insertRule('.aga-card { z-index: ' + (mapZIndexMax + 1) + ';}');
  stylesheet.insertRule('.aga-header { z-index: ' + (mapZIndexMax + 2) + ';}');
}

// Servers with a * require an id option.
const tileServer = {
  ARCGIS: 'ArcGIS world topo',
  CARTODB: 'CartoDB light *',
  MAPBOX: 'MapBox *',
  OSM: 'OpenStreetMap',
  STAMEN: 'Stamen Design *'
};

// Add a base map from the above selection of tile servers.
// If the pane parameter is not null, the layer is assigned to this pane.
function getTileLayer(
  options = { server: tileServer.MAPBOX, id: 'mapbox.streets' }
) {
  let serverUrl = '';
  let tileOptions = { maxZoom: 18 };

  switch (options.server) {
    case tileServer.ARCGIS:
      serverUrl =
        'http://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}';
      tileOptions.attribution =
        'Map tiles © <a href="https://www.arcgis.com">ArcGIS</a> contributors. ' +
        'Map data © <a href="http://osm.org/copyright">OpenStreetMap</a> contributors, ' +
        'under <a href="http://creativecommons.org/licenses/by-sa/3.0">CC BY SA</a>. ' +
        'The GIS User Community.'; // Note that this attribution should actually be collected from the copyrightText field in http://services.arcgisonline.com/arcgis/rest/services/World_Topo_Map/MapServer?f=pjson
      break;
    case tileServer.CARTODB:
      // Alternative URL: 'https://{s}.basemaps.cartocdn.com/{id}/{z}/{x}/{y}.png'
      serverUrl =
        'https://cartodb-basemaps-{s}.global.ssl.fastly.net/{id}/{z}/{x}/{y}.png';
      tileOptions.attribution =
        'Map tiles from <a href="http://cartodb.com/attributions">CartoDB</a>, ' +
        'under <a href="http://creativecommons.org/licenses/by/3.0">CC BY 3.0</a>. ' +
        'Map data © <a href="https://www.openstreetmap.org/">OpenStreetMap</a> contributors, ' +
        'under <a href="https://creativecommons.org/licenses/by-sa/3.0/">CC-BY-SA</a>.';
      // Example IDs: 'light_all', 'light_nolabels', 'light_only_labels'.
      break;
    case tileServer.MAPBOX:
      serverUrl =
        'https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw';
      tileOptions.attribution =
        'Map tiles © <a href="https://www.mapbox.com/about/maps/">Mapbox</a>. ' +
        'Map data © <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors, ' +
        'under <a href="http://creativecommons.org/licenses/by-sa/3.0">CC BY SA</a>. ' +
        '<strong><a href="https://www.mapbox.com/map-feedback/" target="_blank">Improve this map</a></strong>';
      // Example IDs: 'mapbox.streets', 'mapbox.mapbox-terrain-v2'.
      break;
    case tileServer.OSM:
      serverUrl = 'http://{s}.tile.osm.org/{z}/{x}/{y}.png';
      tileOptions.attribution =
        'Map data © <a href="http://osm.org/copyright">OpenStreetMap</a> contributors, ' +
        'under <a href="http://creativecommons.org/licenses/by-sa/3.0">CC BY SA</a>.';
      break;
    case tileServer.STAMEN:
      serverUrl = 'http://{s}.tile.stamen.com/{id}/{z}/{x}/{y}.jpg';
      tileOptions.attribution =
        'Map tiles © <a href="http://stamen.com">Stamen Design</a>, ' +
        'under <a href="http://creativecommons.org/licenses/by/3.0">CC BY 3.0</a>. ' +
        'Map data © <a href="http://openstreetmap.org">OpenStreetMap</a>, ' +
        'under <a href="http://creativecommons.org/licenses/by-sa/3.0">CC BY SA</a>.';
      // Example IDs: 'watercolor'.
      break;
    default:
      return null;
  }

  if (options.id !== undefined && options.id !== null)
    tileOptions.id = options.id;

  if (options.pane !== undefined && options.pane !== null)
    tileOptions.pane = options.pane;

  return L.tileLayer(serverUrl, tileOptions);
}

// Extension of L.GridLayer that draws a border around each tile and shows the tile coordinates.
L.GridLayer.DebugGridLayer = L.GridLayer.extend({
  createTile: function(coords) {
    let tile = document.createElement('div');
    tile.innerHTML = [coords.x, coords.y, coords.z].join(', ');
    tile.style.outline = '1px solid lightblue';
    return tile;
  }
});

L.gridLayer.debugCoords = function(opts) {
  return new L.GridLayer.DebugGridLayer(opts);
};
