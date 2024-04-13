// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {getRequiredElement} from 'chrome://resources/js/util.js';

// import {throttlingBrowserProxy} from './throttling_browser_proxy.js';

// function getProxy(): throttlingBrowserProxy {
//   return throttlingBrowserProxy.getInstance();
// }

function initialize() {
  
  const networkDiv = getRequiredElement('network');

  const form = document.createElement('form');
  form.addEventListener('submit', (event) => {
    event.preventDefault(); // Prevent the form from submitting

    const formData = new FormData(form);
    // const latency = parseFloat(formData.get('latency') as string);
    // const downloadThroughput = parseFloat(formData.get('downloadThroughput') as string);
    // const uploadThroughput = parseFloat(formData.get('uploadThroughput') as string);
    
    // const packetLoss = latency*(uploadThroughput+downloadThroughput);
    // const packetQueueLength = Math.floor(packetLoss/2);
    // const packetReordering = false;

      console.log(formData)
    // Use the proxy to set network conditions
    // getProxy().setNetworkConditions(false,latency, downloadThroughput, uploadThroughput,packetLoss, packetQueueLength,  packetReordering);
  });

  form.innerHTML = `
    <label for="latency">Latency:</label>
    <input type="number" id="latency" name="latency" step="0.1" required><br>

    <label for="downloadThroughput">Download Throughput:</label>
    <input type="number" id="downloadThroughput" name="downloadThroughput" step="0.1" required><br>

    <label for="uploadThroughput">Upload Throughput:</label>
    <input type="number" id="uploadThroughput" name="uploadThroughput" step="0.1" required><br>

    <button type="submit">Set Network Conditions</button>
  `;

  networkDiv.appendChild(form);
}

document.addEventListener('DOMContentLoaded', initialize);