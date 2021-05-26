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
let attribute = 'value';
let svg_output = '';

/**
 * Given a data file, return a list of attribute names in it.
 */
function parseColumns(result) {
  let lines = result.split('\n');
  let token = 0;
  let num_col = 0;

  let colNames = [];
  for (let line of lines) {
    let columns = line.split(/ +/);

    for (let column of columns) {
      token++;
      if (token == 1) continue;
      else if (token == 2) num_col = column.length - 1;
      else {
        let col_str = escape(column.replace(/\r/, ''));
        colNames.push(col_str);
      }
    }

    if (2 + num_col <= token) break;
  }

  return colNames;
}

/**
 * Given a data file, initialize the attribute dropdown with the available
 * attribute names. This excludes the attributes 'ID' and 'name'.
 */
function setColumnList(result) {
  const columns = parseColumns(result);
  let list = '';
  for (let column of columns) {
    if (column && column !== 'ID' && column !== 'name') {
      list += '<option value="' + column + '">' + column + '</option>';
    }
  }

  document.getElementById('data_value_in').innerHTML = list;
}

/**
 * Given a data file and the name of one of its attributes, put the data values
 * from that attribute into the input fields.
 *
 * This assumes that the input fields for the regions defined in the data file
 * already exist (see populateDataEditor()).
 */
function parseDataIntoFields(result, attribute) {
  let fields = result.trim().split(/[ \n]+/);
  const cols = parseColumns(result);
  const colValueId = cols.indexOf(attribute);

  let i = 2 + cols.length;
  while (i < fields.length) {
    const id = fields[i].trim();
    const value = fields[i + colValueId];
    document.getElementById('data-editor-editor-' + id).value = value;
    i += cols.length;
  }
}

/**
 * The main initialization function.
 */
function initNecklaceMap() {
  focusSupportCard();
  tryAddSettingsCard('page/settings_necklace.html', 'settings_necklace');

  necklace_geometry_base64 = null;
  necklace_data_base64 = null;
  if (document.getElementById('geometry_file_in') !== null) {
    geometry_file_in.value = '';
  }
  if (document.getElementById('data_file_in') !== null) {
    data_file_in.value = '';
  }
}

/**
 * Callback for when the user selected a different map from the dropdown.
 *
 * This performs an AJAX call to the server to obtain a list of regions in the
 * selected map.
 */
function onChangedMap() {
  const field = document.getElementById('map_choice');
  const selected = field.options[field.selectedIndex].value;
  updateFormState();
  if (selected !== "custom") {

    // download the map
    ajaxGet('data/necklace_map/' + selected + '.svg', function (data) {
      necklace_geometry_base64 = btoa(data);
      necklace_data_base64 = null;

      let params = JSON.stringify({
        geometry_base64: necklace_geometry_base64,
      });
      ajaxPost('/script/run_map_regions.php', params, populateDataEditor);
      region_focused = false;

      updateFormState();
    });
  }
}

/**
 * Callback for when the user selected a different map with the custom map
 * control.
 *
 * This performs an AJAX call to the server to obtain a list of regions in the
 * selected map.
 */
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
    region_focused = false;

    updateFormState();
  };
  reader.readAsArrayBuffer(file);
}

/**
 * Given a newline-separated list of regions, this initializes the data editor
 * by creating an input field for each region.
 */
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

/**
 * Ensures that the correct parts of the form are hidden or made visible,
 * depending on where the user is in the process.
 *
 * More precisely:
 *
 *  - The custom map control is shown iff the custom map option is selected
 *    in the map dropdown.
 *
 *  - If no map has been specified, the remaining parts of the form are hidden.
 *    Otherwise, the algorithm is run.
 *
 *  - If no data has been specified, the remaining parts of the form are
 *    hidden.
 */
function updateFormState() {

  // if the custom map option has been set, show the file picker
  const field = document.getElementById('map_choice');
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

  runNecklaceAlgorithm();

  // if we have no data yet, hide the next parts of the form
  if (!necklace_data_base64) {
    document.getElementById('options_panel').style.display = 'none';
    return;
  }
  document.getElementById('options_panel').style.display = 'block';

  if (!svg_output) {
    document.getElementById('output_panel').style.display = 'none';
    return;
  }
  document.getElementById('output_panel').style.display = 'block';
}

/**
 * Callback for when the user selected a data file in the file picker. This
 * reads the file to be able to initialize the attributes list.
 */
function onChangedDataFile() {
  const fileField = document.getElementById('data_file_in');
  const file = fileField.files[0];

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
    setColumnList(result);
  };
  reader.readAsArrayBuffer(file);
}

/**
 * Callback for when the user clicks the Import button. This reads the file
 * and puts the values into the input fields.
 */
function onImportButtonClicked() {
  const fileField = document.getElementById('data_file_in');
  const file = fileField.files[0];

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
    attribute = document.getElementById('data_value_in').value;
    parseDataIntoFields(result, attribute);
    updateFormState();
  };
  reader.readAsArrayBuffer(file);
}

/**
 * Callback for when the user clicks the Save data values button. This reads
 * the contents of the input fields, and generates a data file based on that.
 */
function onSaveButtonClicked() {
  const editor = document.getElementById('data-editor');
  let data = editor.childNodes.length + " sd\nID value\n";
  for (let i = 0; i < editor.childNodes.length; i++) {
    const child = editor.childNodes[i];
    const id = child.childNodes[0].textContent;
    let value = child.childNodes[1].childNodes[0].value;
    if (!value) {
      value = 0;
    }
    data += id + " " + value + "\n";
  }
  necklace_data_base64 = btoa(data);
  attribute = "value";

  updateFormState();
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

/**
 * Generates a function replacing the Leaflet map by the output from the
 * algorithm.
 */
function processNecklaceMapResponse() {
  return function (response) {
    replaceMapBySvgResponse(!region_focused)(response);
    region_focused = true;
    svg_output = response;
  };
}

/**
 * Callback for when the user changed any settings.
 *
 * This handles placing the setting values in the output fields, and updating
 * the explanation for the interval setting.
 *
 * Afterwards, it reruns the algorithm with the updated settings.
 */
function onChangedNecklaceSettings() {
  if (necklace_geometry_base64 === null) return;

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

  if (document.getElementById('interval_in').value === "centroid") {
    document.getElementById('interval-explanation-image').src = 'res/bead-placement-centroid.svg';
    document.getElementById('interval-explanation-centroid').style.display = 'block';
    document.getElementById('interval-explanation-wedge').style.display = 'none';
  } else {  // wedge
    document.getElementById('interval-explanation-image').src = 'res/bead-placement-wedge.svg';
    document.getElementById('interval-explanation-centroid').style.display = 'none';
    document.getElementById('interval-explanation-wedge').style.display = 'block';
  }

  runNecklaceAlgorithm();
}

/**
 * Performs an AJAX request to the server to run the actual algorithm.
 *
 * If the data has not been specified yet, no AJAX call is made, and instead
 * the input SVG map is used as output.
 */
function runNecklaceAlgorithm() {

  if (!necklace_data_base64) {
    let necklace_geometry = atob(necklace_geometry_base64);
    svg_output = '';
    processNecklaceMapResponse()(necklace_geometry);
    setColumnList('');
    return;
  }

  if (Date.now() - last_update < UPDATE_MILLISECONDS) return;
  last_update = Date.now();

  // Collect the necklace map parameters.
  let body = JSON.stringify({
    geometry_base64: necklace_geometry_base64,
    data_base64: necklace_data_base64,
    value: attribute,
    interval: document.getElementById('interval_in').value,
    ignore_point_regions: ignore_point_regions_in.checked,
    order: order_in.value,
    centroid_interval_length: parseFloat(getNecklaceCentroidIntervalLength()),
    wedge_interval_length_min: parseFloat(getNecklaceWedgeIntervalLengthMin()),
    buffer_rad: parseFloat(getNecklaceBuffer()),
    aversion_ratio: parseFloat(getNecklaceAversion()),
    bead_id_font_size: parseFloat(getNecklaceBeadIdSize()),
  });

  // Run the necklace map PHP script on the server.
  ajaxPost('/script/run_necklace_map.php', body, processNecklaceMapResponse());
}

function onClickedDownload() {
  alert(svg_output);
  /*const svg_blob = Blob(svg_output)
  const url = URL.createObjectURL(svg_blob);
  const a = document.createElement('a');
  a.download = 'map.svg';
  a.href = url;
  a.click();*/
  // todo remove
}

