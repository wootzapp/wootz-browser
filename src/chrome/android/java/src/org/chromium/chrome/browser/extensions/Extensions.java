package org.chromium.chrome.browser.extensions;

import org.jni_zero.NativeMethods;
import java.util.ArrayList;

public class Extensions {
    public static ArrayList<ExtensionInfo> getExtensionsInfo() {
        return ExtensionsJni.get().getExtensionsInfo();
    }

    @NativeMethods
    interface Natives {
        ArrayList<ExtensionInfo> getExtensionsInfo();
    }
}