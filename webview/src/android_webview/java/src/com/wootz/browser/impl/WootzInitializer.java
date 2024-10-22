package com.wootz.browser.impl;

import org.chromium.android_webview.AwBrowserProcess;
import org.chromium.android_webview.AwResource;
import org.chromium.content.browser.ResourceExtractor;
import org.chromium.content.common.CommandLine;

import com.wootz.browser.WootzView;
import android.content.Context;

/**
 * Chromium setup chores.
 */
public class WootzInitializer {
  private static final String[] MANDATORY_PAKS = { "webviewchromium.pak" };

  /**
   * The entry point to the initialization process.
   *
   * This is called by {@link WootzView#initialize(Context)}.
   *
   * @param context Android context for the application using WootzView
   */
  public static void initialize(Context context) {
    if (initializeCalled_) {
      return;
    }
    initializeCalled_ = true;

    // Initialization lifted from
    //     chromium/src/android_webview/test/shell/src/org/chromium/android_webview/shell/AwShellResourceProvider

    AwResource.setResources(context.getResources());

    AwResource.RAW_LOAD_ERROR = com.wootz.browser.R.raw.blank_html;
    AwResource.RAW_NO_DOMAIN = com.wootz.browser.R.raw.blank_html;

    AwResource.STRING_DEFAULT_TEXT_ENCODING =
        com.wootz.browser.R.string.default_encoding;

    // Initialization lifted from
    //     chromium/src/android_webview/test/shell/src/org/chromium/android_webview/shell/AwShellApplication

    CommandLine.initFromFile("/data/local/Wootz-command-line");

    ResourceExtractor.setMandatoryPaksToExtract(MANDATORY_PAKS);
    ResourceExtractor.setExtractImplicitLocaleForTesting(false);
    AwBrowserProcess.loadLibrary();
    AwBrowserProcess.start(context);
  }

  /** Ensures that initialize() is only called once. */
  private static boolean initializeCalled_ = false;
}
