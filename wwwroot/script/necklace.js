/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const MAX_FILE_SIZE_BYTES = 100 * 1024;

let necklace_geometry_base64 = null;
let necklace_data_base64 = null;
let region_focused = false;
let column_list = null;

function setColumnList(result) {
  let lines = result.split('\n');
  let token = 0;
  let num_col = 0;

  let list = '';
  for (let line of lines) {
    let columns = line.split(/ +/);

    for (let column of columns) {
      token++;
      if (token == 1) continue;
      else if (token == 2) num_col = column.length - 1;
      else {
        let col_str = escape(column.replace(/\r/, ''));
        if (col_str != 'ID' && col_str != 'name' && col_str != 'null')
          list += '<option value="' + col_str + '">' + col_str + '</option>';
      }
    }

    if (2 + num_col <= token) break;
  }

  document.getElementById('data_value_in').innerHTML = list;
}

function initNecklaceMap() {
  focusSupportCard();
  tryAddSettingsCard('page/settings_necklace.html', 'settings_necklace');

  necklace_geometry_base64 = null;
  necklace_data_base64 = null;
  if (document.getElementById('geometry_file_in') !== null)
    geometry_file_in.value = '';
  if (document.getElementById('data_file_in') !== null)
    data_file_in.value = '';
}

function onChangedGeometryFile(file) {
  if (!file) {
    return;
  }

  // check mime type and file size
  if (file.type !== 'text/xml' && file.type !== 'image/svg+xml') {
    alert('The map needs to be in SVG format');
    return;
  }
  if (MAX_FILE_SIZE_BYTES < file.size) {
    alert('The map needs to be at most ' + MAX_FILE_SIZE_BYTES + 'bytes');
    return;
  }

  // when uploading a new map, reset the data file
  necklace_data_base64 = null;
  data_file_in.value = '';

  // read the map
  var reader = new FileReader();
  reader.onload = function (e) {
    necklace_geometry_base64 = btoa(
      String.fromCharCode(...new Uint8Array(e.target.result))
    );

    let params = JSON.stringify({
      geometry_base64: necklace_geometry_base64,
    });
    ajaxPost('/script/run_map_regions.php', params, populateDataEditor);

    updateFormState();
  };
  reader.readAsArrayBuffer(file);
}

function populateDataEditor(response) {
  const regions = response.trim().split('\n');

  const editor = document.getElementById('data-editor');
  editor.textContent = '';
  for (let i = 0; i < regions.length; i++) {
    const name = regions[i];
    const row = document.createElement('li');
    const nameCell = document.createElement('span');
    nameCell.classList.add('region-label');
    const nameText = document.createTextNode(name);
    nameCell.appendChild(nameText);
    row.appendChild(nameCell);
    const inputCell = document.createElement('span');
    const input = document.createElement('input');
    input.classList.add('region-field');
    input.id = 'data-editor-editor-' + name;
    inputCell.appendChild(input);
    row.appendChild(inputCell);
    editor.appendChild(row);
  }
}

function updateFormState() {

  // if the custom map option has been set, show the file picker
  const field = document.getElementById('geometry_choice');
  const selected = field.options[field.selectedIndex].value;
  if (selected === "custom") {
    document.getElementById('geometry_file_in').style.display = 'inline-block';
  } else {
    document.getElementById('geometry_file_in').style.display = 'none';
  }

  // if we have no map yet, hide the next parts of the form
  if (!necklace_geometry_base64) {
    document.getElementById('data_panel').style.display = 'none';
    document.getElementById('options_panel').style.display = 'none';
    return;
  }
  document.getElementById('data_panel').style.display = 'block';
}

function onChangedDataFile(file) {
  if (!file) {
    return;
  }

  // check mime type and file size
  if (file.type != 'text/plain') {
    alert('Data files need to be plain text files');
    return;
  }
  if (MAX_FILE_SIZE_BYTES < file.size) {
    alert('Data file needs to be at most ' + MAX_FILE_SIZE_BYTES + 'bytes');
    return;
  }

  // read the file
  var reader = new FileReader();
  reader.onload = function (e) {
    let result = String.fromCharCode(...new Uint8Array(e.target.result));
    necklace_data_base64 = btoa(result);
    setColumnList(result);
    updateFormState();
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

function getNecklaceCentroidIntervalLength() {
  let length = parseFloat(centroid_interval_length_in.value);
  return (length * Math.PI).toPrecision(4);
}

function getNecklaceWedgeIntervalLengthMin() {
  let length = parseFloat(wedge_interval_length_min_in.value);
  return (length * Math.PI).toPrecision(4);
}

function getNecklaceBeadIdSize() {
  let size = parseFloat(bead_id_font_size_in.value);
  return size.toPrecision(4);
}

function processNecklaceMapResponse() {
  return function (response) {
    replaceMapBySvgResponse(!region_focused)(response);
    region_focused = true;
    //geometry_out.value = response;
    document.getElementById('output_panel').style.display = 'block';
  };
}

function onChangedNecklaceSettings() {
  if (necklace_geometry_base64 === null) return;

  if (document.getElementById('interval_in').value === "centroid") {
    document.getElementById('interval-explanation-image').src = 'res/bead-placement-centroid.svg';
    document.getElementById('interval-explanation-centroid').style.display = 'block';
    document.getElementById('interval-explanation-wedge').style.display = 'none';
  } else {  // wedge
    document.getElementById('interval-explanation-image').src = 'res/bead-placement-wedge.svg';
    document.getElementById('interval-explanation-centroid').style.display = 'none';
    document.getElementById('interval-explanation-wedge').style.display = 'block';
  }

  if (necklace_data_base64 === null) {
    let necklace_geometry = atob(necklace_geometry_base64);
    processNecklaceMapResponse()(necklace_geometry);

    // The geometry has not been processed yet.
    //geometry_out.value = '';
    document.getElementById('output_panel').style.display = 'none';
    setColumnList('');

    return;
  }

  if (Date.now() - last_update < UPDATE_MILLISECONDS) return;
  last_update = Date.now();

  let value_str = escape(data_value_in.value);
  if (value_str == '') return;

  // Collect the necklace map parameters.
  let body = JSON.stringify({
    geometry_base64: necklace_geometry_base64,
    data_base64: necklace_data_base64,
    value: escape(data_value_in.value),
    interval: document.getElementById('interval_in').value,
    ignore_point_regions: ignore_point_regions_in.checked,
    order: order_in.value,
    centroid_interval_length: parseFloat(getNecklaceCentroidIntervalLength()),
    wedge_interval_length_min: parseFloat(getNecklaceWedgeIntervalLengthMin()),
    buffer_rad: parseFloat(getNecklaceBuffer()),
    aversion_ratio: parseFloat(getNecklaceAversion()),
    bead_id_font_size: parseFloat(getNecklaceBeadIdSize()),
  });

  //geometry_out.value = '';
  document.getElementById('output_panel').style.display = 'none';

  // Run the necklace map PHP script on the server.
  ajaxPost('/script/run_necklace_map.php', body, processNecklaceMapResponse());
}

function onInputNecklaceSettings() {
  let buffer_rad = getNecklaceBuffer();
  let glyph_aversion = getNecklaceAversion();
  let centroid_interval_length = getNecklaceCentroidIntervalLength();
  let wedge_interval_length_min = getNecklaceWedgeIntervalLengthMin();
  let bead_id_font_size = getNecklaceBeadIdSize();
  buffer_rad_out.value = '= ' + buffer_rad;
  aversion_out.value = '= ' + glyph_aversion;
  centroid_interval_length_out.value = '= ' + centroid_interval_length;
  wedge_interval_length_min_out.value = '= ' + wedge_interval_length_min;
  bead_id_font_size_out.value = '= ' + parseInt(bead_id_font_size);
  onChangedNecklaceSettings();
}
