package com.wootz.browser.impl;

import org.chromium.android_webview.JsPromptResultReceiver;

import com.wootz.browser.WootzJsPromptResult;
import com.wootz.browser.WootzJsResult;

/**
 * Proxies from android_webkit's JsResultReceiver to JsPromptResult.
 *
 * @hide
 */
public class WootzJsPromptResultProxy
    implements WootzJsResult.ResultReceiver {

  /** The proxy target. */
  private JsPromptResultReceiver target_;

  public WootzJsPromptResultProxy(JsPromptResultReceiver target) {
    target_ = target;
  }
  @Override
  public void onJsResultComplete(WootzJsResult result) {
    WootzJsPromptResult promptResult = (WootzJsPromptResult)result;
    if (result.getResult()) {
      target_.confirm(promptResult.getStringResult());
    } else {
      target_.cancel();
    }
  }
}
