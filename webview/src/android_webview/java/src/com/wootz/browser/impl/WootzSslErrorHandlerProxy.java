package com.wootz.browser.impl;

import com.wootz.browser.WootzSslErrorHandler;
import android.webkit.ValueCallback;

/**
 * Proxies from WootzSslErrorHandler to ValueCallback<Boolean>.
 *
 * @hide
 */
public class WootzSslErrorHandlerProxy implements WootzSslErrorHandler {
  /** The proxy target. */
  private ValueCallback<Boolean> target_;

  public WootzSslErrorHandlerProxy(ValueCallback<Boolean> target) {
    target_ = target;
  }
  @Override
  public void cancel() {
    target_.onReceiveValue(false);
  }
  @Override
  public void proceed() {
    target_.onReceiveValue(true);
  }
}
