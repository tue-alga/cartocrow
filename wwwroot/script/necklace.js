/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const MAX_FILE_SIZE_BYTES = 100 * 1024;

function getNecklaceBuffer() {
  let buffer_rad = parseFloat(buffer_rad_in.value);
  return (Math.pow(buffer_rad, 4) * Math.PI).toPrecision(4);
}

function getNecklaceAversion() {
  let aversion = parseFloat(aversion_in.value);
  return Math.max(Math.pow(aversion, 4).toPrecision(4), 0.001);
}

function onChangedNecklaceSettings() {
  if (Date.now() - last_update < UPDATE_MILISECONDS) return;
  if (geometry_in.value == '' || data_in.value == '') return;

  const geometry_str = geometry_in.value;
  const data_str = data_in.value;

  last_update = Date.now();
  let buffer_rad = getNecklaceBuffer();
  let glyph_aversion = getNecklaceAversion();
  tryReplaceMapBySvg(
    '/script/run_necklace_map.php?args=--out_website|' +
      '--buffer_rad:' +
      buffer_rad +
      '|--aversion_ratio:' +
      glyph_aversion
  );
}

function onInputNecklaceSettings() {
  let buffer_rad = getNecklaceBuffer();
  let glyph_aversion = getNecklaceAversion();
  buffer_rad_out.value = '= ' + buffer_rad;
  aversion_out.value = '= ' + glyph_aversion;
  onChangedNecklaceSettings();
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

  // Read the file contents.
  var reader = new FileReader();
  reader.onload = function(e) {
    geometry_in.value = e.target.result;
    onChangedNecklaceSettings();
  };
  reader.readAsText(file);
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
    data_in.value = e.target.result;
    onChangedNecklaceSettings();
  };
  reader.readAsText(file);
}

function InitNecklaceMap() {
  //tryReplaceMapBySvg('/script/run_necklace_map.php?args=--out_website');
  focusSupportCard();
  tryAddSettingsCard('/page/settings_necklace.html', 'settings_necklace');
}
