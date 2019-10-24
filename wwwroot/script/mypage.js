/*
    Copyright 2019 Netherlands eScience Center
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const SUPPORT_CONTAINER_ID = 'support_cards';
const SUPPORT_CARD_CLASS = 'mypage-support';
const SUPPORT_CARD_INNER_CLASS = 'mypage-support-inner';

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
  return function(response) {
    // Add the response content into the content item.
    document.getElementById(cardId).children[0].innerHTML = response;
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
      '" class="myorg-panel myorg-card ' +
      SUPPORT_CARD_CLASS +
      '" style="height: 0;">' +
      '<div class=' +
      SUPPORT_CARD_INNER_CLASS +
      '><div class="myorg-fill myorg-center"><p>Loading content...</p></div></div>' +
      '</section>';

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsSupportCard(cardId));
  }

  if (gainFocus) focusSupportCard(cardId);
}

// Initialize the map block using Leaflet.
function initMap() {
  let map = L.map('map').fitWorld();

  // Use CartoDB without labels as base layer.
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

  // Enable to overlay a grid on the map.
  //L.gridLayer.debugCoords().addTo(map);

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
  stylesheet.insertRule('.myorg-card { z-index: ' + (mapZIndexMax + 1) + ';}');
  stylesheet.insertRule(
    '.myorg-header { z-index: ' + (mapZIndexMax + 2) + ';}'
  );

  return map;
}

// Callback function to add the svg direct child elements in the response text to the map.
function addSvgResponseToMap(map) {
  return function(response) {
    // Add the response text as a new element.
    let wrapperElement = document.createElement('div');
    wrapperElement.innerHTML = response;

    // Add all svg children of the wrapper element to the map.
    for (let child of wrapperElement.children) {
      if (child.tagName === 'svg') {
        // Note that children without a parsable "bounds" attribute are added but not shown.
        let bounds = [[0, 0], [0, 0]];
        if (child.hasAttribute('bounds'))
          try {
            bounds = JSON.parse(child.getAttribute('bounds'));
          } catch {}
        L.svgOverlay(child, bounds).addTo(map);
      }
    }
  };
}

// Add a SVG glyph to the map.
function tryAddSvgToMap(url, map) {
  // Get the item content from an external URL.
  ajaxGet(url, addSvgResponseToMap(map));
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
