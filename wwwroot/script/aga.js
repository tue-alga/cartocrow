/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

let nav_id = 'nav';

// Toggle the menu for small screens.
function toggleNavigation(force_hide = true) {
  let nav = document.getElementById(nav_id);
  if (nav.className.indexOf('aga-hide-small') == -1) {
    nav.className += ' aga-hide-small';
  } else if (!force_hide) {
    nav.className = nav.className.replace(' aga-hide-small', '');
  }
}

// Generic AJAX function with GET method.
function ajaxGet(url, callback) {
  let request = new XMLHttpRequest();
  request.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      callback(this);
    }
  };
  request.open('GET', url, true);
  request.send();
}
