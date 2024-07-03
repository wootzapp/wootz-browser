import {$} from 'chrome://resources/js/util.js';
import {BrowserBridge} from './browser_bridge.js';
import {addNode, addNodeWithText} from './util.js';
import {DivView} from './view.js';

/** @type {?ThrottleView} */
let instance = null;

/**
 * This UI allows a user to set network throttling conditions such as offline status,
 * latency, and throughput, to simulate different network environments.
 */
export class ThrottleView extends DivView {
  constructor() {
    super(ThrottleView.MAIN_BOX_ID);

    this.browserBridge_ = BrowserBridge.getInstance();
    this.offlineSelect_ = $(ThrottleView.OFFLINE_SELECT_ID);
    this.latencyInput_ = $(ThrottleView.LATENCY_INPUT_ID);
    this.downloadThroughputInput_ = $(ThrottleView.DOWNLOAD_THROUGHPUT_INPUT_ID);
    this.uploadThroughputInput_ = $(ThrottleView.UPLOAD_THROUGHPUT_INPUT_ID);
    this.packetLossInput_ = $(ThrottleView.PACKET_LOSS_INPUT_ID);
    this.packetQueueLengthInput_ = $(ThrottleView.PACKET_QUEUE_LENGTH_INPUT_ID);
    this.savedSettingsDiv_ = $(ThrottleView.SAVED_SETTINGS_ID);

    let form = $(ThrottleView.FORM_ID);
    form.addEventListener('submit', this.onSubmit_.bind(this), false);

    // Set up listener for saved settings
    window.cr.addWebUiListener("displaySavedSettings", (settings) => {
      this.displaySavedSettings_(settings);
    });

    // Fetch and display the saved network throttling settings on load
    this.loadAndDisplaySavedSettings_();
  }

  async onSubmit_(event) {
    event.preventDefault();
    const offline = this.offlineSelect_.value === 'true';
    const latency = parseFloat(this.latencyInput_.value.trim());
    const downloadThroughput = parseFloat(this.downloadThroughputInput_.value.trim());
    const uploadThroughput = parseFloat(this.uploadThroughputInput_.value.trim());
    const packetLoss = parseFloat(this.packetLossInput_.value.trim()) || 0;
    const packetQueueLength = parseInt(this.packetQueueLengthInput_.value.trim(), 10) || 0;

    await this.browserBridge_.sendSetNetworkThrottling({
      offline,
      latency,
      downloadThroughput,
      uploadThroughput,
      packetLoss,
      packetQueueLength,
    });

    this.loadAndDisplaySavedSettings_();
  }

  async loadAndDisplaySavedSettings_() {
    const settings = await this.browserBridge_.getNetworkThrottlingSettings();
    this.displaySavedSettings_(settings);
  }

  displaySavedSettings_(settings) {
    const {offline, latency, downloadThroughput, uploadThroughput, packetLoss, packetQueueLength} = settings;
    this.savedSettingsDiv_.textContent = `
      Offline: ${offline}
      Latency: ${latency} ms
      Download Throughput: ${downloadThroughput} Bps
      Upload Throughput: ${uploadThroughput} Bps
      Packet Loss: ${packetLoss}%
      Packet Queue Length: ${packetQueueLength}
    `;
  }

  static getInstance() {
    return instance || (instance = new ThrottleView());
  }
}

// IDs for special HTML elements in throttle_view.html
ThrottleView.MAIN_BOX_ID = 'throttle-view-tab-content';
ThrottleView.FORM_ID = 'throttle-form';
ThrottleView.OFFLINE_SELECT_ID = 'throttle-offline-select';
ThrottleView.LATENCY_INPUT_ID = 'throttle-latency-input';
ThrottleView.DOWNLOAD_THROUGHPUT_INPUT_ID = 'throttle-download-throughput-input';
ThrottleView.UPLOAD_THROUGHPUT_INPUT_ID = 'throttle-upload-throughput-input';
ThrottleView.PACKET_LOSS_INPUT_ID = 'throttle-packet-loss-input';
ThrottleView.PACKET_QUEUE_LENGTH_INPUT_ID = 'throttle-packet-queue-length-input';
ThrottleView.SAVED_SETTINGS_ID = 'throttle-saved-settings';
ThrottleView.SUBMIT_ID = 'throttle-submit';