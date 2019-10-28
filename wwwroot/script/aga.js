/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const nav_id = 'nav';

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
  const init = {
    method: 'GET',
    headers: {
      'Content-Type': 'text/html'
    }
  };
  let request = new Request(url);
  fetch(request, init)
    .then(response => {
      if (response.status == 200) {
        return response.text();
      }
      throw new Error(
        'Unexpected response status in ajaxGet: ' + response.status
      );
    })
    .then(callback, console.error);
}
