import {addWebUiListener} from 'chrome://resources/js/cr.js';

window.addEventListener('load', function() {
    addWebUiListener('displaySavedSettings', (settings) => {
        getRequiredElement('throttle-saved-settings').innerText = settings;
      });
});