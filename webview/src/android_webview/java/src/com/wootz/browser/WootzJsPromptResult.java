package com.wootz.browser;

/**
 * WootzView equivalent of JsPromptResult.
 *
 * This is necessary because JsPromptResult.ResultRecever is hidden in the SDK.
 *
 * @see android.webkit.JsPromptResult
 */
public class WootzJsPromptResult extends WootzJsResult {
  // Mostly mirrors
  //    platform/frameworks/base/ ./core/java/android/webkit/JsPromptResult

  // String result of the prompt
  private String mStringResult;

  /**
   * Handle a confirmation response from the user.
   */
  public void confirm(String result) {
      mStringResult = result;
      confirm();
  }

  /**
   * @hide Only for use by WebViewProvider implementations
   */
  public WootzJsPromptResult(ResultReceiver receiver) {
      super(receiver);
  }

  /**
   * @hide Only for use by WebViewProvider implementations
   */
  public String getStringResult() {
      return mStringResult;
  }
}
