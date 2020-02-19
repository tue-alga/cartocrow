L.Control.NecklaceSettings = L.Control.extend({
  options: {
    position: 'bottomleft'
  },
  onAdd: function(map) {
    let control = L.DomUtil.create('div');
    control.innerHTML =
      '<div class="geoviz-map-control aga-card aga-stack-5 aga-theme-main">' +
      '<p>Glyph Separation Settings</p>' +
      '<form style="display: block;">' +
      '<div oninput="onInputNecklaceSettings();" onchange="onChangedNecklaceSettings();">' +
      '<input id="buffer_rad_in" type="range" name="buffer_rad" min="0.0" max="1.0" step="0.01" value="0.0" />' +
      '<label for="buffer_rad" style="margin-left: 8px;">Buffer</label>' +
      '<output id="buffer_rad_out" name="buffer_rad" for="buffer_rad"></output>' +
      '</div>' +
      '<div oninput="onInputNecklaceSettings();" onchange="onChangedNecklaceSettings();">' +
      '<input id="aversion_in" type="range" name="aversion" min="0.0" max="1.0" step="0.01" value="0.0" list="aversion_extremes"/>' +
      '<label for="aversion" style="margin-left: 8px;">Aversion</label>' +
      '<output id="aversion_out" name="aversion" for="aversion"></output>' +
      '</div>' +
      '</form>' +
      '</div>';

    L.DomEvent.disableClickPropagation(control);
    return control;
  },

  onRemove: function(map) {}
});

L.control.neclaceSettings = function(opts) {
  return new L.Control.NecklaceSettings(opts);
};
