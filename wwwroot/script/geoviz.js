/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const SETTINGS_CONTAINER_ID = 'settings_cards';
const SETTINGS_CARD_CLASS = 'aga-panel-left';
const SETTINGS_CARD_INNER_CLASS = 'aga-panel-left-inner';
const SUPPORT_CONTAINER_ID = 'support_cards';
const SUPPORT_CARD_CLASS = 'aga-panel-top';
const SUPPORT_CARD_INNER_CLASS = 'aga-panel-top-inner';

let map_obj;
let world_map = false;
let last_update = Date.now();
const UPDATE_MILISECONDS = 50;

function unfocusAllExcept(cardId = null) {
  let card = document.getElementById(cardId);
  for (let c of document.getElementsByClassName(SETTINGS_CARD_CLASS)) {
    if (c !== card) {
      c.children[1].style.width = '0%';
      c.style.width = '0%';
    }
  }
  for (let c of document.getElementsByClassName(SUPPORT_CARD_CLASS)) {
    if (c !== card) {
      c.children[0].style.height = '0%';
      c.style.height = '0%';
    }
  }
}

// Focus on a specific settings card.
function focusSettingsCard(cardId = null) {
  unfocusAllExcept(cardId);
  if (cardId !== null) {
    let card = document.getElementById(cardId);
    card.children[1].style.width = '100%';
    card.style.width = '35%';
  } else {
    toggleNavigation();
  }
}

// Focus on a specific support card.
function focusSupportCard(cardId = null) {
  unfocusAllExcept(cardId);
  if (cardId !== null) {
    let card = document.getElementById(cardId);
    card.children[0].style.height = '100%';
    card.style.height = '100%';
  } else {
    toggleNavigation();
  }
}

// Toggle the settings panel.
function toggleSettings(cardId) {
  let card = document.getElementById(cardId);
  let inner = card.children[1];
  let icon_child = card.children[0];
  let icon = icon_child.children[0].children[0];
  if (card.className.indexOf('aga-hide-side-panel') == -1) {
    card.className += ' aga-hide-side-panel';
    card.style.width = icon_child.style.width;
    icon.className = icon.className.replace('fa-caret-left', 'fa-caret-right');
  } else {
    card.className = card.className.replace(' aga-hide-side-panel', '');
    card.style.width = '35%';
    icon.className = icon.className.replace('fa-caret-right', 'fa-caret-left');
  }
}

// Callback function to add the response text as card.
function setResponseAsCard(cardId, child = 0) {
  return function(response) {
    // Add the response content into the content item.
    document.getElementById(cardId).children[child].innerHTML = response;
  };
}

// Add a settings card and open it.
function tryAddSettingsCard(url, cardId, gainFocus = true) {
  // Always disable the menu.
  toggleNavigation();

  // Check whether the element already exists.
  const element = document.getElementById(cardId);
  if (element === null) {
    // Create a new section element in the container that indicates that the
    // content is loading.
    // Note that the initial width is set to 0 to hide the card.
    let cardIdStr = "'" + cardId + "'";
    let cardInnerId = cardId + '-inner';
    document.getElementById(SETTINGS_CONTAINER_ID).innerHTML +=
      '<section id="' +
      cardId +
      '" class="aga-panel aga-card aga-stack-4 aga-theme-meta ' +
      SETTINGS_CARD_CLASS +
      '" style="width: 0;">' +
      '<div class="aga-center aga-panel-left-minimize"><a title="Toggle settings menu" href="javascript:void(0)" ' +
      'onclick="toggleSettings(' +
      cardIdStr +
      ')"' +
      ' class="aga-button aga-center aga-theme-meta">' +
      '<i class="fa fa-caret-left"></i>' +
      '</a></div>' +
      '<div id="' +
      cardInnerId +
      '" class=' +
      SETTINGS_CARD_INNER_CLASS +
      '>' +
      '<div><div class="aga-fill aga-center"><p>Loading content...</p></div></div>' +
      '</div>' +
      '</section>';

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsCard(cardInnerId));
  }

  if (gainFocus) focusSettingsCard(cardId);
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
      '" class="aga-panel aga-card aga-stack-3 aga-theme-meta ' +
      SUPPORT_CARD_CLASS +
      '" style="height: 0;">' +
      '<div class=' +
      SUPPORT_CARD_INNER_CLASS +
      '><div class="aga-fill aga-center"><p>Loading content...</p></div></div>' +
      '</section>';

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsCard(cardId));
  }

  if (gainFocus) focusSupportCard(cardId);
}

// Initialize the map block using Leaflet.
function initMap() {
  if (world_map) return;

  if (map_obj !== undefined) map_obj.remove();
  map_obj = L.map('map').fitWorld();

  // Use CartoDB without labels as base layer.
  getTileLayer({ server: tileServer.CARTODB, id: 'light_nolabels' }).addTo(
    map_obj
  );
  world_map = true;

  // Add CartoDB labels as separate pane.
  map_obj.createPane('labels');
  let paneLabels = map_obj.getPane('labels');
  paneLabels.style.zIndex = 650; // On top of markers but below pop-ups
  paneLabels.style.pointerEvents = 'none'; // Disable mouse events on this particular pane.
  getTileLayer({
    server: tileServer.CARTODB,
    id: 'light_only_labels',
    pane: 'labels'
  }).addTo(map_obj);

  // Enable to overlay a grid on the map.
  //L.gridLayer.debugCoords().addTo(map_obj);

  // Add a scale control.
  L.control.scale().addTo(map_obj);

  function onLocationFound(e) {
    // Focus on the user's location.
    const radius = e.accuracy / 2;

    L.marker(e.latlng)
      .addTo(map_obj)
      .bindPopup('You are within ' + radius + ' meters from this point')
      .openPopup();

    L.circle(e.latlng, radius).addTo(map_obj);
  }

  function onLocationError(e) {
    // Focus on TU/e.
    map_obj.setView([51.448, 5.49], 16);
  }

  // Try to locate the user.
  map_obj.on('locationfound', onLocationFound);
  map_obj.on('locationerror', onLocationError);
  map_obj.locate({ setView: true, maxZoom: 16 });

  // L.marker([51.448, 5.49]).addTo(map_obj);

  /*L.circle([51.448, 5.49], {
      color: "red",
      fillColor: "#f03",
      fillOpacity: 0.5,
      radius: 500
    }).addTo(map_obj);*/

  // L.polygon([[51.509, -0.08], [51.503, -0.06], [51.51, -0.047]]).addTo(map_obj);

  // Make sure that the navigation and support cards always show on top of the map.
  // Note that Leaflet takes care of the control z-index.
  const mapZIndexMax = Math.max(
    getComputedStyle(document.querySelector('.leaflet-control')).zIndex,
    getComputedStyle(document.querySelector('.leaflet-top')).zIndex
  );
  let stylesheet = document.styleSheets[document.styleSheets.length - 1];
  stylesheet.insertRule('.aga-stack-4 { z-index: ' + (mapZIndexMax + 1) + ';}');
  stylesheet.insertRule('.aga-stack-3 { z-index: ' + (mapZIndexMax + 2) + ';}');
  stylesheet.insertRule('.aga-stack-2 { z-index: ' + (mapZIndexMax + 3) + ';}');
  stylesheet.insertRule('.aga-stack-1 { z-index: ' + (mapZIndexMax + 4) + ';}');

  // Add a SVG glyph generated by a server-side executable to the map.
  // This demonstrates how to run server-side C++ code.
  tryAddSvgToMap('/script/run_draw_logo.php');
}

// Callback function to add the SVG direct child elements in the response text to the map.
function addSvgResponseToMap() {
  return function(response) {
    // Add the response text as a new element.
    let wrapperElement = document.createElement('div');
    wrapperElement.innerHTML = response;

    // Add all svg children of the wrapper element to the map.
    for (let child of wrapperElement.children) {
      if (child.tagName === 'svg') {
        // Note that children without a parsable "bounds" attribute are added but not shown.
        let bounds = [
          [0, 0],
          [0, 0]
        ];
        if (child.hasAttribute('bounds'))
          try {
            bounds = JSON.parse(child.getAttribute('bounds'));
          } catch {}
        L.svgOverlay(child, bounds).addTo(map_obj);
      }
    }
  };
}

// Replace the map by a SVG glyph.
function tryAddSvgToMap(url) {
  // Get the item content from an external URL.
  ajaxGet(url, addSvgResponseToMap());
}

// Some additional cleanup on destroying a SVG container.
L.SVG.include({
  _destroyContainer: function() {
    L.DomUtil.remove(this._container);
    L.DomEvent.off(this._container);
    delete this._container;
    delete this._rootGroup;

    // Make sure to also clear the cache for svgSize,
    // so that next container width and height will be set.
    delete this._svgSize;
  }
});

// Callback function to replace the map by the SVG direct child elements in the response text.
function replaceMapBySvgResponse() {
  return function(response) {
    let was_world = world_map;
    if (world_map) {
      map_obj.remove();
      map_obj = L.map('map', { crs: L.CRS.Simple });
      //L.control.neclaceSettings().addTo(map_obj);
      world_map = false;
    } else {
      map_obj.eachLayer(function(thisLayer) {
        map_obj.removeLayer(thisLayer);
      });

      //TODO(tvl) probably replace by explicitly removing added SVG layers...
      //if (svgLayer) { svgLayer.remove(); }
    }

    // Add the response text as a new element.
    let wrapperElement = document.createElement('div');
    wrapperElement.innerHTML = response;

    // Collect the bounds from the response text.
    let map_bounds = [
      [Infinity, Infinity],
      [-Infinity, -Infinity]
    ];
    for (let child of wrapperElement.children) {
      if (child.tagName === 'svg') {
        if (child.hasAttribute('bounds'))
          try {
            let bound = JSON.parse(child.getAttribute('bounds'));
            map_bounds[0][0] = Math.min(map_bounds[0][0], bound[0][0]);
            map_bounds[0][0] = Math.min(map_bounds[0][0], bound[1][0]);

            map_bounds[0][1] = Math.min(map_bounds[0][1], bound[0][1]);
            map_bounds[0][1] = Math.min(map_bounds[0][1], bound[1][1]);

            map_bounds[1][0] = Math.max(map_bounds[1][0], bound[0][0]);
            map_bounds[1][0] = Math.max(map_bounds[1][0], bound[1][0]);

            map_bounds[1][1] = Math.max(map_bounds[1][1], bound[0][1]);
            map_bounds[1][1] = Math.max(map_bounds[1][1], bound[1][1]);
          } catch {}
      }
    }
    if (map_bounds[0][0] === Infinity) return;

    if (was_world) map_obj.fitBounds(map_bounds);

    // Add all svg children of the wrapper element to the map.
    for (let child of wrapperElement.children) {
      if (child.tagName === 'svg') {
        // Note that children without a parsable "bounds" attribute are added but not shown.
        let bounds = [
          [0, 0],
          [0, 0]
        ];
        if (child.hasAttribute('bounds'))
          try {
            bounds = JSON.parse(child.getAttribute('bounds'));
          } catch {}
        L.svgOverlay(child, bounds).addTo(map_obj);
      }
    }
  };
}

// Replace the map by a SVG glyph.
function tryReplaceMapBySvg(url) {
  // Get the item content from an external URL.
  ajaxGet(url, replaceMapBySvgResponse());
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
