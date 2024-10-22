package com.wootz.browser.impl;

import org.chromium.android_webview.AwHttpAuthHandler;

import com.wootz.browser.WootzHttpAuthHandler;

/**
 * Proxies from WootzHttpAuthHandler to AwHttpAuthHandler.
 *
 * @hide
 */
public class WootzHttpAuthHandlerProxy implements WootzHttpAuthHandler {
  /** The proxy target. */
  private AwHttpAuthHandler target_;

  public WootzHttpAuthHandlerProxy(AwHttpAuthHandler target) {
    target_ = target;
  }
  @Override
  public boolean useHttpAuthUsernamePassword() {
    return false;
  }
  @Override
  public void cancel() {
    target_.cancel();
  }
  @Override
  public void proceed(String username, String password) {
    target_.proceed(username, password);
  }
  @Override
  public boolean suppressDialog() {
    return false;
  }
}
