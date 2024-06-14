package org.chromium.chrome.browser.vpn.wireguard;

import org.chromium.build.annotations.IdentifierNameString;
import org.chromium.chrome.browser.base.SplitCompatService;

/** See {@link WireguardServiceImpl}. */
public class WireguardService extends SplitCompatService {
    @IdentifierNameString
    private static String sImplClassName =
            "org.chromium.chrome.browser.vpn.wireguard.WireguardServiceImpl";

    public WireguardService() {
        super(sImplClassName);
    }
}