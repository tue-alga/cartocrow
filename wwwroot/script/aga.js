/*
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
*/

'use strict';

const NAVIGATION_MENU_ID = 'nav';

const SETTINGS_CONTAINER_ID = 'settings_cards';
const SETTINGS_CARD_CLASS = 'aga-panel-left';
const SETTINGS_CARD_INNER_CLASS = 'aga-panel-left-inner';

const SUPPORT_CONTAINER_ID = 'support_cards';
const SUPPORT_CARD_CLASS = 'aga-panel-top';
const SUPPORT_CARD_INNER_CLASS = 'aga-panel-top-inner';

const UPDATE_MILLISECONDS = 50;

let last_update = Date.now();

// Toggle the menu for small screens.
function toggleNavigation(force_hide = true) {
  let nav = document.getElementById(NAVIGATION_MENU_ID);
  if (nav.className.indexOf('aga-hide-small') == -1) {
    nav.className += ' aga-hide-small';
  } else if (!force_hide) {
    nav.className = nav.className.replace(' aga-hide-small', '');
  }
}

// Generic AJAX request with callback.
function ajaxMethod(url, init, callback) {
  let request = new Request(url);
  fetch(request, init)
    .then((response) => {
      if (response.status == 200) {
        return response.text();
      }
      throw new Error(
        'Unexpected response status in ajax call: ' + response.status
      );
    })
    .then(callback, console.error);
}

// AJAX function with GET method.
function ajaxGet(url, callback) {
  const init = {
    method: 'GET',
    headers: {
      'Content-Type': 'text/html',
    },
  };
  ajaxMethod(url, init, callback);
}

// AJAX function with POST method.
function ajaxPost(url, body, callback) {
  const init = {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: body,
  };
  ajaxMethod(url, init, callback);
}

function unfocusAllExcept(cardId = null) {
  let card = document.getElementById(cardId);
  for (let c of document.getElementsByClassName(SETTINGS_CARD_CLASS)) {
    if (c !== card) {
      c.children[1].style.width = '0%';
      c.style.width = '0%';

      c.className = c.className.replace(' aga-hide-side-panel', '');
      let icon = c.children[0].children[0].children[0];
      icon.className = icon.className.replace(
        'fa-caret-right',
        'fa-caret-left'
      );
    }
  }
  for (let c of document.getElementsByClassName(SUPPORT_CARD_CLASS)) {
    if (c !== card) {
      c.children[0].style.height = '0%';
      c.style.height = '0%';
    }
  }
}

// Focus on a specific settings card.
function focusSettingsCard(cardId = null) {
  unfocusAllExcept(cardId);
  if (cardId !== null) {
    let card = document.getElementById(cardId);
    card.children[1].style.width = '100%';
    card.style.width = '35%';
  } else {
    toggleNavigation();
  }
}

// Focus on a specific support card.
function focusSupportCard(cardId = null) {
  unfocusAllExcept(cardId);
  if (cardId !== null) {
    let card = document.getElementById(cardId);
    card.children[0].style.height = '100%';
    card.style.height = '100%';
  } else {
    toggleNavigation();
  }
}

// Toggle the settings panel.
function toggleSettings(cardId) {
  //TODO(tvl) add toggle settings on focus support
  let card = document.getElementById(cardId);
  let inner = card.children[1];
  let icon_child = card.children[0];
  let icon = icon_child.children[0].children[0];
  if (card.className.indexOf('aga-hide-side-panel') == -1) {
    card.className += ' aga-hide-side-panel';
    card.style.width = icon_child.style.width;
    icon.className = icon.className.replace('fa-caret-left', 'fa-caret-right');
  } else {
    card.className = card.className.replace(' aga-hide-side-panel', '');
    card.style.width = '35%';
    icon.className = icon.className.replace('fa-caret-right', 'fa-caret-left');
  }
}

// Callback function to add the response text as card.
function setResponseAsCard(cardId, child = 0) {
  return function (response) {
    // Add the response content into the content item.
    document.getElementById(cardId).children[child].innerHTML = response;
  };
}

// Add a settings card and open it.
function tryAddSettingsCard(url, cardId, gainFocus = true) {
  // Always disable the menu.
  toggleNavigation();

  // Check whether the element already exists.
  const element = document.getElementById(cardId);
  if (element === null) {
    // Create a new section element in the container that indicates that the
    // content is loading.
    // Note that the initial width is set to 0 to hide the card.
    let cardIdStr = "'" + cardId + "'";
    let cardInnerId = cardId + '-inner';
    document.getElementById(SETTINGS_CONTAINER_ID).innerHTML +=
      '<section id="' +
      cardId +
      '" class="aga-panel aga-card aga-stack-4 aga-theme-meta ' +
      SETTINGS_CARD_CLASS +
      '" style="width: 0;">' +
      '<div class="aga-center aga-panel-left-minimize"><a title="Toggle settings menu" href="javascript:void(0)" ' +
      'onclick="toggleSettings(' +
      cardIdStr +
      ')"' +
      ' class="aga-button aga-center aga-theme-meta">' +
      '<i class="fa fa-caret-left"></i>' +
      '</a></div>' +
      '<div id="' +
      cardInnerId +
      '" class=' +
      SETTINGS_CARD_INNER_CLASS +
      '>' +
      '<div><div class="aga-fill aga-center"><p>Loading content...</p></div></div>' +
      '</div>' +
      '</section>';

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsCard(cardInnerId));
  }

  if (gainFocus) focusSettingsCard(cardId);
}

// Add a support card and open it.
function tryAddSupportCard(url, cardId, gainFocus = true) {
  // Always disable the menu.
  toggleNavigation();

  // Check whether the element already exists.
  const element = document.getElementById(cardId);
  if (element === null) {
    // Create a new section element in the container that indicates that the
    // content is loading.
    // Note that the initial height is set to 0 to hide the card.
    document.getElementById(SUPPORT_CONTAINER_ID).innerHTML +=
      '<section id="' +
      cardId +
      '" class="aga-panel aga-card aga-stack-3 aga-theme-meta ' +
      SUPPORT_CARD_CLASS +
      '" style="height: 0;">' +
      '<div class=' +
      SUPPORT_CARD_INNER_CLASS +
      '><div class="aga-fill aga-center"><p>Loading content...</p></div></div>' +
      '</section>';

    // Get the item content from an external URL.
    ajaxGet(url, setResponseAsCard(cardId));
  }

  if (gainFocus) focusSupportCard(cardId);
}
