package com.wootz.browser.impl;

import org.chromium.android_webview.JsResultReceiver;

import com.wootz.browser.WootzJsResult;

/**
 * Proxies from android_webkit's JsResultReceiver to JsResult.
 *
 * @hide
 */
public class WootzJsResultReceiverProxy implements
    WootzJsResult.ResultReceiver {

  /** The proxy target. */
  private JsResultReceiver target_;

  public WootzJsResultReceiverProxy(JsResultReceiver target) {
    target_ = target;
  }
  @Override
  public void onJsResultComplete(WootzJsResult result) {
    if (result.getResult()) {
      target_.confirm();
    } else {
      target_.cancel();
    }
  }
}
