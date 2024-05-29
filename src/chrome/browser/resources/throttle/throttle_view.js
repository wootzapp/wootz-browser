import {$} from 'chrome://resources/js/util.js';
import {BrowserBridge} from './browser_bridge.js';
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
    this.packetReorderingSelect_ = $(ThrottleView.PACKET_REORDERING_SELECT_ID);

    this.savedOffline_ = $(ThrottleView.SAVED_OFFLINE_ID);
    this.savedLatency_ = $(ThrottleView.SAVED_LATENCY_ID);
    this.savedDownloadThroughput_ = $(ThrottleView.SAVED_DOWNLOAD_THROUGHPUT_ID);
    this.savedUploadThroughput_ = $(ThrottleView.SAVED_UPLOAD_THROUGHPUT_ID);
    this.savedPacketLoss_ = $(ThrottleView.SAVED_PACKET_LOSS_ID);
    this.savedPacketQueueLength_ = $(ThrottleView.SAVED_PACKET_QUEUE_LENGTH_ID);

    let form = $(ThrottleView.FORM_ID);
    form.addEventListener('submit', this.onSubmit_.bind(this), false);

    // Fetch and display the saved network throttling settings on load
    this.browserBridge_.getNetworkThrottlingSettings().then(this.displaySavedSettings_.bind(this));
  }

  onSubmit_(event) {
    event.preventDefault();
    const offline = this.offlineSelect_.value === 'true';
    const latency = parseFloat(this.latencyInput_.value.trim());
    const downloadThroughput = parseFloat(this.downloadThroughputInput_.value.trim());
    const uploadThroughput = parseFloat(this.uploadThroughputInput_.value.trim());
    const packetLoss = parseFloat(this.packetLossInput_.value.trim()) || undefined;
    const packetQueueLength = parseInt(this.packetQueueLengthInput_.value.trim(), 10) || undefined;
    const packetReordering = this.packetReorderingSelect_.value === 'true';

    this.browserBridge_.sendSetNetworkThrottling({
      offline,
      latency,
      downloadThroughput,
      uploadThroughput,
      packetLoss,
      packetQueueLength,
      packetReordering
    });

    // Update the displayed saved settings
    this.displaySavedSettings_({
      offline,
      latency,
      downloadThroughput,
      uploadThroughput,
      packetLoss,
      packetQueueLength
    });
  }

  displaySavedSettings_(settings) {
    this.savedOffline_.textContent = 'Offline:' + (settings.offline ? 'Yes' : 'No');
    this.savedLatency_.textContent = 'Latency: ' + settings.latency + ' ms';
    this.savedDownloadThroughput_.textContent = 'Download Throughput: ' + settings.downloadThroughput + ' bytes/sec';
    this.savedUploadThroughput_.textContent = 'Upload Throughput: ' + settings.uploadThroughput + ' bytes/sec';
    this.savedPacketLoss_.textContent = 'Packet Loss: ' + settings.packetLoss + ' %';
    this.savedPacketQueueLength_.textContent = 'Packet Queue Length: ' + settings.packetQueueLength;
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
ThrottleView.PACKET_REORDERING_SELECT_ID = 'throttle-packet-reordering-select';
ThrottleView.SUBMIT_ID = 'throttle-submit';

ThrottleView.SAVED_OFFLINE_ID = 'saved-offline';
ThrottleView.SAVED_LATENCY_ID = 'saved-latency';
ThrottleView.SAVED_DOWNLOAD_THROUGHPUT_ID = 'saved-download-throughput';
ThrottleView.SAVED_UPLOAD_THROUGHPUT_ID = 'saved-upload-throughput';
ThrottleView.SAVED_PACKET_LOSS_ID = 'saved-packet-loss';
ThrottleView.SAVED_PACKET_QUEUE_LENGTH_ID = 'saved-packet-queue-length';
