"use strict";

// Toggle the menu for small screens.
// TODO(tvl) remove w3
function toggleMenu() {
  var x = document.getElementById("navMobile");
  if (x.className.indexOf("w3-show") == -1) {
    x.className += " w3-show";
  } else {
    x.className = x.className.replace(" w3-show", "");
  }
}

// Focus on a specific content item.
function focusItem(id) {
  for (let c of document.getElementsByClassName("content-item"))
    c.style.display = "none";
  document.getElementById(id).style.display = "block";
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

// Callback function to add the response text as item of the content container.
function addResponseAsContentItem(id, focus) {
  return function(request) {
    // Add the request content as section element in the container.
    document.getElementById("content").innerHTML +=
      '<section id="' +
      id +
      '" class="content-item">' +
      request.responseText +
      "</section>";

    // Make the item fill the remaining height of the container.
    let item = document.getElementById(id);
    item.children[0].style.height = "100%";
    item.children[0].style.boxSizing = "border-box";

    // Set focus.
    if (focus) focusItem(id);
    else item.style.display = "none";
  };
}

// Add a section to the section container.
function tryAddSection(url, id, focus = true) {
  // Check whether the element already exists;
  // if so, just set focus to the item.
  let element = document.getElementById(id);
  if (element === null) {
    ajaxGet(url, addResponseAsContentItem(id, focus));
  } else if (focus) focusItem(id);
  else element.style.display = "none";
}
