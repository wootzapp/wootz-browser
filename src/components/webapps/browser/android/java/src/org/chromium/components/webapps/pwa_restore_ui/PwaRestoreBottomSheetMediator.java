// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.webapps.pwa_restore_ui;

import android.app.Activity;
import android.text.TextUtils;
import android.util.Pair;
import android.view.View;

import org.chromium.components.webapps.R;
import org.chromium.components.webapps.pwa_restore_ui.PwaRestoreProperties.ViewState;
import org.chromium.ui.modelutil.PropertyModel;

import java.util.ArrayList;
import java.util.List;

/** The Mediator for the PWA Restore bottom sheet. */
class PwaRestoreBottomSheetMediator {
    // The current activity.
    private final Activity mActivity;

    // The underlying property model for the bottom sheeet.
    private final PropertyModel mModel;

    PwaRestoreBottomSheetMediator(
            ArrayList recentApps,
            ArrayList olderApps,
            Activity activity,
            Runnable onReviewButtonClicked,
            Runnable onBackButtonClicked) {
        mActivity = activity;
        mModel =
                PwaRestoreProperties.createModel(
                        onReviewButtonClicked,
                        onBackButtonClicked,
                        this::onDeselectButtonClicked,
                        this::onRestoreButtonClicked,
                        this::onSelectionToggled);

        initializeState(recentApps, olderApps);
        setPeekingState();
    }

    private void initializeState(ArrayList recentApps, ArrayList olderApps) {
        mModel.set(
                PwaRestoreProperties.PEEK_TITLE,
                mActivity.getString(R.string.pwa_restore_title_peeking));
        mModel.set(
                PwaRestoreProperties.PEEK_DESCRIPTION,
                mActivity.getString(R.string.pwa_restore_description_peeking));
        mModel.set(
                PwaRestoreProperties.PEEK_BUTTON_LABEL,
                mActivity.getString(R.string.pwa_restore_button_peeking));

        mModel.set(
                PwaRestoreProperties.EXPANDED_TITLE,
                mActivity.getString(R.string.pwa_restore_title_expanded));
        mModel.set(
                PwaRestoreProperties.EXPANDED_DESCRIPTION,
                mActivity.getString(R.string.pwa_restore_description_expanded));
        mModel.set(
                PwaRestoreProperties.RECENT_APPS_TITLE,
                mActivity.getString(R.string.pwa_restore_recent_apps_list));
        mModel.set(
                PwaRestoreProperties.OLDER_APPS_TITLE,
                mActivity.getString(R.string.pwa_restore_older_apps_list));
        mModel.set(
                PwaRestoreProperties.EXPANDED_BUTTON_LABEL,
                mActivity.getString(R.string.pwa_restore_button_expanded));
        mModel.set(
                PwaRestoreProperties.DESELECT_BUTTON_LABEL,
                mActivity.getString(R.string.pwa_restore_button_deselect));

        mModel.set(PwaRestoreProperties.APPS, Pair.create(recentApps, olderApps));
    }

    protected void setPeekingState() {
        mModel.set(PwaRestoreProperties.VIEW_STATE, ViewState.PREVIEW);
    }

    protected void setPreviewState() {
        mModel.set(PwaRestoreProperties.VIEW_STATE, ViewState.VIEW_PWA_LIST);
    }

    private void onDeselectButtonClicked() {
        Pair<List<PwaRestoreProperties.AppInfo>, List<PwaRestoreProperties.AppInfo>> appLists =
                mModel.get(PwaRestoreProperties.APPS);
        // Deselect all recent apps.
        for (PwaRestoreProperties.AppInfo app : appLists.first) {
            if (app.isSelected()) app.toggleSelection();
        }
        // Deselect all older apps.
        for (PwaRestoreProperties.AppInfo app : appLists.second) {
            if (app.isSelected()) app.toggleSelection();
        }
        mModel.set(PwaRestoreProperties.APPS, appLists);
    }

    private void onRestoreButtonClicked() {
        // TODO(finnur): Implement.
    }

    private void onSelectionToggled(View view) {
        String appId = (String) view.getTag();

        Pair<List<PwaRestoreProperties.AppInfo>, List<PwaRestoreProperties.AppInfo>> appLists =
                mModel.get(PwaRestoreProperties.APPS);

        for (PwaRestoreProperties.AppInfo app : appLists.first) {
            if (TextUtils.equals(app.getId(), appId)) {
                app.toggleSelection();
                return;
            }
        }

        for (PwaRestoreProperties.AppInfo app : appLists.second) {
            if (TextUtils.equals(app.getId(), appId)) {
                app.toggleSelection();
                return;
            }
        }
    }

    PropertyModel getModel() {
        return mModel;
    }
}
