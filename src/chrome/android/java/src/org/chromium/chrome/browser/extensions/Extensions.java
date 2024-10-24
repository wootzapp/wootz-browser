package org.chromium.chrome.browser.extensions;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import android.content.Context;
import androidx.appcompat.app.AlertDialog;
import java.util.ArrayList;

public class Extensions {
    public static ArrayList<ExtensionInfo> getExtensionsInfo() {
        return ExtensionsJni.get().getExtensionsInfo();
    }

    public static void uninstallExtension(String extensionId) {
        ExtensionsJni.get().uninstallExtension(extensionId);
    }

    @NativeMethods
    interface Natives {
        ArrayList<ExtensionInfo> getExtensionsInfo();
        void uninstallExtension(String extensionId);
    }
}