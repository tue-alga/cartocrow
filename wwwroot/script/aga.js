"use strict";

let nav_id = "nav";

// Toggle the menu for small screens.
function toggleNavigation(force_hide = true) {
  let x = document.getElementById(nav_id);
  if (x.className.indexOf("aga-hide-small") == -1) {
    x.className += " aga-hide-small";
  } else if (!force_hide) {
    x.className = x.className.replace(" aga-hide-small", "");
  }
}

// Generic AJAX function with GET method.
function ajaxGet(url, callback) {
  var request;
  request = new XMLHttpRequest();
  request.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      callback(this);
    }
  };
  request.open("GET", url, true);
  request.send();
}
