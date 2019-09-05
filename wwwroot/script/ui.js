"use strict";

let content_item_class = "content-item";

// Toggle the menu for small screens.
function toggleNavigation(nav_id, force = true) {
  let x = document.getElementById(nav_id);
  if (x.className.indexOf("aga-hide-small") == -1) {
    x.className += " aga-hide-small";
  } else if (!force) {
    x.className = x.className.replace(" aga-hide-small", "");
  }
}

// Focus on a specific content item.
function focusContentItem(content_item_id) {
  for (let c of document.getElementsByClassName(content_item_class))
    c.style.display = "none";
  document.getElementById(content_item_id).style.display = "flex";
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
function addResponseAsContentItem(
  content_container_id,
  content_item_id,
  focus
) {
  return function(request) {
    // Add the request content as section element in the container.
    document.getElementById(content_container_id).innerHTML +=
      '<section id="' +
      content_item_id +
      '" class="' +
      content_item_class +
      '">' +
      request.responseText +
      "</section>";

    // Make the item fill the remaining height of the container.
    let item = document.getElementById(content_item_id);
    item.children[0].style.height = "100%";
    item.children[0].style.boxSizing = "border-box";

    // Set focus.
    if (focus) focusContentItem(content_item_id);
    else item.style.display = "none";
  };
}

// Add a section to the section container.
function tryAddSection(
  url,
  content_container_id,
  content_item_id,
  focus = true
) {
  // Check whether the element already exists;
  // if so, just set focus to the item.
  let element = document.getElementById(content_item_id);
  if (element === null) {
    ajaxGet(
      url,
      addResponseAsContentItem(content_container_id, content_item_id, focus)
    );
  } else if (focus) focusContentItem(content_item_id);
  else element.style.display = "none";
}
