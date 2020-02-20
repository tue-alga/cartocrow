/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const MAX_FILE_SIZE_BYTES = 100 * 1024;

let necklace_geometry_base64 = null;
let necklace_data_base64 = null;

function initNecklaceMap() {
  focusSupportCard();
  tryAddSettingsCard('/page/settings_necklace.html', 'settings_necklace');

  necklace_geometry_base64 = null;
  necklace_data_base64 = null;
  if (document.getElementById('geometry_file_in') !== null)
    geometry_file_in.value = '';
  if (document.getElementById('data_file_in') !== null) data_file_in.value = '';
}

function onChangedGeometryFile(file) {
  // Check mime type and file size.
  if (file.type != 'text/xml') {
    alert('XML file type required.');
    return;
  }
  if (MAX_FILE_SIZE_BYTES < file.size) {
    alert('File size restricted to ' + MAX_FILE_SIZE_BYTES + 'bytes');
    return;
  }

  // Reset the data file.
  necklace_data_base64 = null;
  data_file_in.value = '';

  // Read the file contents.
  var reader = new FileReader();
  reader.onload = function(e) {
    necklace_geometry_base64 = btoa(
      String.fromCharCode(...new Uint8Array(e.target.result))
    );
    onChangedNecklaceSettings();
  };
  reader.readAsArrayBuffer(file);
}

function onChangedDataFile(file) {
  // Check mime type and file size.
  if (file.type != 'text/plain') {
    alert('Plain text file type required.');
    return;
  }
  if (MAX_FILE_SIZE_BYTES < file.size) {
    alert('File size restricted to ' + MAX_FILE_SIZE_BYTES + 'bytes');
    return;
  }

  // Read the file contents.
  var reader = new FileReader();
  reader.onload = function(e) {
    necklace_data_base64 = btoa(
      String.fromCharCode(...new Uint8Array(e.target.result))
    );
    onChangedNecklaceSettings();
  };
  reader.readAsArrayBuffer(file);
}

function getNecklaceBuffer() {
  let buffer_rad = parseFloat(buffer_rad_in.value);
  return (Math.pow(buffer_rad, 4) * Math.PI).toPrecision(4);
}

function getNecklaceAversion() {
  let aversion = parseFloat(aversion_in.value);
  return Math.max(Math.pow(aversion, 4).toPrecision(4), 0.001);
}

function processNecklaceMapResponse() {
  return function(response) {
    replaceMapBySvgResponse()(response);
    geometry_out.value = response;
  };
}

function onChangedNecklaceSettings() {
  if (Date.now() - last_update < UPDATE_MILLISECONDS) return;
  if (necklace_geometry_base64 === null || necklace_data_base64 === null)
    return;

  last_update = Date.now();

  // Collect the necklace map parameters.
  let body = JSON.stringify({
    geometry_base64: necklace_geometry_base64,
    data_base64: necklace_data_base64,
    value: escape(data_value_in.value),
    buffer_rad: parseFloat(getNecklaceBuffer()),
    aversion_ratio: parseFloat(getNecklaceAversion())
  });

  // Run the necklace map PHP script on the server.
  //ajaxPost('/script/run_necklace_map.php', body, replaceMapBySvgResponse());
  ajaxPost('/script/run_necklace_map.php', body, processNecklaceMapResponse());
}

function onInputNecklaceSettings() {
  let buffer_rad = getNecklaceBuffer();
  let glyph_aversion = getNecklaceAversion();
  buffer_rad_out.value = '= ' + buffer_rad;
  aversion_out.value = '= ' + glyph_aversion;
  onChangedNecklaceSettings();
}
