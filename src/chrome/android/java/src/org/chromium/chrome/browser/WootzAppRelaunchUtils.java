/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;
import org.chromium.chrome.R;

@JNINamespace("chrome::android")
public class WootzAppRelaunchUtils {
    static public void askForRelaunch(Context context) {
        if (context == null) return;
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
        alertDialogBuilder
            .setMessage("Relaunch Now")
            .setCancelable(true)
            .setPositiveButton("Relaunch", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog,int id) {
                    MisesRelaunchUtilsJni.get().restart();
                    dialog.cancel();
                }
            })
            .setNegativeButton("Relaunch Later",new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog,int id) {
                    dialog.cancel();
                }
            });
            AlertDialog alertDialog = alertDialogBuilder.create();
            alertDialog.show();
    }

    public static void askForRelaunchCustom(Context context) {
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
        alertDialogBuilder.setTitle(R.string.reset_needed_error_title)
                .setMessage(R.string.reset_needed_error_description)
                .setCancelable(true)
                .setPositiveButton(R.string.settings_require_relaunch_now,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int id) {
                                MisesRelaunchUtilsJni.get().restart();
                                dialog.cancel();
                            }
                        })
                .setNegativeButton(R.string.settings_require_relaunch_later,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.cancel();
                            }
                        });
        AlertDialog alertDialog = alertDialogBuilder.create();
        alertDialog.show();
    }

    public static void restart() {
        MisesRelaunchUtilsJni.get().restart();
    }

    @NativeMethods
    interface Natives {
        void restart();
    }
}