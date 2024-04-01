import {getTrustedHTML} from 'chrome://resources/js/static_types.js';
import {getRequiredElement} from 'chrome://resources/js/util.js';

import type {ClientInfo} from './hello_world.mojom-webui.js';
import {HelloWalletBrowserProxy} from './hello_world_browser_proxy.js';

function getProxy(): HelloWalletBrowserProxy {
  return HelloWalletBrowserProxy.getInstance();
}

function addClientInfo(parent: HTMLElement, info: ClientInfo) {
  const div = document.createElement('div');
  div.className = 'client';
  const configTitle = document.createElement('h5');
 
  parent.appendChild(div);
}



function initialize() {
  

  getProxy().getCallbackRouter().onClientInfoAvailable.addListener(
      (clientInfos: ClientInfo[]) => {
        const parent = getRequiredElement('client-container');
        // Remove all current children.
        while (parent.firstChild) {
          parent.removeChild(parent.firstChild);
        }
        // Append new children.
        for (let i = 0; i < clientInfos.length; ++i) {
          addClientInfo(parent, clientInfos[i]!);
        }
      });

  getProxy().getServiceStatus();
}

document.addEventListener('DOMContentLoaded', initialize);