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
  request.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      callback(this);
    }
  };
  request.open("GET", url, true);
  request.send();
}

// Callback function to add the response text as item of the content container.
function setResponseAsContentItem(content_item_id) {
  return function (request) {
    // Add the request content into the content item.
    document.getElementById(content_item_id).innerHTML = request.responseText;
  };
}

// Add a section to the section container.
function tryAddSection(
  url,
  content_container_id,
  content_item_id
) {
  // Check whether the element already exists.
  let element = document.getElementById(content_item_id);
  if (element === null) {
    // Create a new section element in the container that indicates that the content is loading.
    document.getElementById(content_container_id).innerHTML +=
      '<section id="' + content_item_id + '" class="' + content_item_class + '">' +
      '<div style="display: flex; justify-content: center; align-items: center;"><p>Loading content...</p></div>' +
      '</section>';

    // Set focus to the item.
    focusContentItem(content_item_id);

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsContentItem(content_item_id));
  } else focusContentItem(content_item_id);
}