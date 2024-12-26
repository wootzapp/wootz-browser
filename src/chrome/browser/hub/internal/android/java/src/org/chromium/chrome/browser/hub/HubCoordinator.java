// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.hub;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.FrameLayout;
import android.graphics.Outline;
import android.view.ViewOutlineProvider;
import android.util.TypedValue;
import android.graphics.Path;  



import android.widget.LinearLayout;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
// import gen._chrome._android._chrome_java__assetres.srcjar.R;

//import java.nio.file.Path;

import org.chromium.base.Callback;
import org.chromium.base.supplier.ObservableSupplier;
import org.chromium.base.supplier.ObservableSupplierImpl;
import org.chromium.base.supplier.TransitiveObservableSupplier;
import org.chromium.chrome.browser.layouts.LayoutType;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.toolbar.menu_button.MenuButtonCoordinator;
import org.chromium.components.browser_ui.widget.gesture.BackPressHandler;
import org.chromium.components.browser_ui.widget.gesture.BackPressHandler.BackPressResult;

import android.graphics.Color;
import android.graphics.Outline;
import android.view.ViewOutlineProvider;


/** Root coordinator of the Hub. */
public class HubCoordinator implements PaneHubController, BackPressHandler {
    private static final Integer START_SURFACE_LAYOUT_TYPE =
            Integer.valueOf(LayoutType.START_SURFACE);

    private final @NonNull FrameLayout mContainerView;
    private final @NonNull View mMainHubParent;
    private final @NonNull PaneManager mPaneManager;
    private final @NonNull HubToolbarCoordinator mHubToolbarCoordinator;
    private final @NonNull HubPaneHostCoordinator mHubPaneHostCoordinator;
    private final @NonNull HubLayoutController mHubLayoutController;
    private final @NonNull ObservableSupplierImpl<Boolean> mHandleBackPressSupplier;
    private final @NonNull HubPaneHostView hubPaneHostView;
    private final @NonNull HubToolbarView hubToolbarView;
    /**
     * Generic callback that invokes {@link #updateHandleBackPressSupplier()}. This can be cast to
     * an arbitrary {@link Callback} and the provided value is discarded.
     */
    private final @NonNull Callback<Object> mBackPressStateChangeCallback;

    /**
     * Warning: {@link #getFocusedPane()} may return null if no pane is focused or {@link
     * Pane#getHandleBackPressChangedSupplier()} contains null.
     */
    private final @NonNull TransitiveObservableSupplier<Pane, Boolean>
            mFocusedPaneHandleBackPressSupplier;

    private final @NonNull PaneBackStackHandler mPaneBackStackHandler;
    private final @NonNull ObservableSupplier<Tab> mCurrentTabSupplier;

    /**
     * Creates the {@link HubCoordinator}.
     *
     * @param containerView The view to attach the Hub to.
     * @param paneManager The {@link PaneManager} for Hub.
     * @param hubLayoutController The controller of the {@link HubLayout}.
     * @param currentTabSupplier The supplier of the current {@link Tab}.
     * @param menuButtonCoordinator Root component for the app menu.
     */
    public HubCoordinator(
            @NonNull FrameLayout containerView,
            @NonNull PaneManager paneManager,
            @NonNull HubLayoutController hubLayoutController,
            @NonNull ObservableSupplier<Tab> currentTabSupplier,
            @NonNull MenuButtonCoordinator menuButtonCoordinator) {
        Context context = containerView.getContext();
        mBackPressStateChangeCallback = (ignored) -> updateHandleBackPressSupplier();
        mPaneManager = paneManager;
        mFocusedPaneHandleBackPressSupplier =
                new TransitiveObservableSupplier<>(
                        paneManager.getFocusedPaneSupplier(),
                        p -> p.getHandleBackPressChangedSupplier());
        mFocusedPaneHandleBackPressSupplier.addObserver(
                castCallback(mBackPressStateChangeCallback));

        mContainerView = containerView;
        mMainHubParent = LayoutInflater.from(context).inflate(R.layout.hub_layout, null);
        mMainHubParent.setId(View.generateViewId());
        mContainerView.addView(mMainHubParent);

        hubToolbarView = mContainerView.findViewById(R.id.hub_toolbar);
        // This might be causing crash whenever you click on Tabswitcher on a native incognito new tab page
        // float borderRadius = 80f;
        // hubToolbarView.setClipToOutline(true);
        // hubToolbarView.setOutlineProvider(new ViewOutlineProvider() {
        //     @Override
        //     public void getOutline(View view, Outline outline) {
        //         outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), borderRadius);
        //     }
        // });

        mHubToolbarCoordinator =
                new HubToolbarCoordinator(hubToolbarView, paneManager, menuButtonCoordinator);

        hubPaneHostView = mContainerView.findViewById(R.id.hub_pane_host);
        mHubPaneHostCoordinator =
                new HubPaneHostCoordinator(hubPaneHostView, paneManager.getFocusedPaneSupplier());
                if (true) {
                    LinearLayout.LayoutParams params = (LinearLayout.LayoutParams)
                    hubPaneHostView.getLayoutParams();
                    params.bottomMargin = hubToolbarView.getHeight();
                    hubPaneHostView.setLayoutParams(params);
                }

        mHubLayoutController = hubLayoutController;
        mHandleBackPressSupplier = new ObservableSupplierImpl<>();

        mPaneBackStackHandler = new PaneBackStackHandler(paneManager);
        mPaneBackStackHandler
                .getHandleBackPressChangedSupplier()
                .addObserver(castCallback(mBackPressStateChangeCallback));

        mCurrentTabSupplier = currentTabSupplier;
        mCurrentTabSupplier.addObserver(castCallback(mBackPressStateChangeCallback));

        mCurrentTabSupplier.addObserver((tab) -> {
            setHubBackgroundColor();
        });
        
        mHubLayoutController
                .getPreviousLayoutTypeSupplier()
                .addObserver(castCallback(mBackPressStateChangeCallback));

        updateHandleBackPressSupplier();
        setupViewForHubPaneHostView();
        setupViewForHubToolbarView();

        setHubBackgroundColor();
    }

    /** Removes the hub from the layout tree and cleans up resources. */
    public void destroy() {
        mContainerView.removeView(mMainHubParent);

        mFocusedPaneHandleBackPressSupplier.removeObserver(
                castCallback(mBackPressStateChangeCallback));
        mCurrentTabSupplier.removeObserver(castCallback(mBackPressStateChangeCallback));
        mHubLayoutController
                .getPreviousLayoutTypeSupplier()
                .removeObserver(castCallback(mBackPressStateChangeCallback));
        mPaneBackStackHandler
                .getHandleBackPressChangedSupplier()
                .removeObserver(castCallback(mBackPressStateChangeCallback));
        mPaneBackStackHandler.destroy();

        mHubToolbarCoordinator.destroy();
        mHubPaneHostCoordinator.destroy();
    }

    @Override
    public @BackPressResult int handleBackPress() {
        if (Boolean.TRUE.equals(mFocusedPaneHandleBackPressSupplier.get())
                && getFocusedPane().handleBackPress() == BackPressResult.SUCCESS) {
            return BackPressResult.SUCCESS;
        }

        if (mPaneBackStackHandler.getHandleBackPressChangedSupplier().get()
                && mPaneBackStackHandler.handleBackPress() == BackPressResult.SUCCESS) {
            return BackPressResult.SUCCESS;
        }

        // TODO(crbug.com/40287515): Discuss with Start Surface owners and investigate removing.
        if (startSurfaceHandlesBackPress()) {
            // This is based on the logic in TabSwitcherMediator where the logic is delegated to
            // ReturnToChromeBackPressHandler.
            return BackPressResult.FAILURE;
        }

        Tab tab = mCurrentTabSupplier.get();
        if (tab != null) {
            mHubLayoutController.selectTabAndHideHubLayout(tab.getId());
            return BackPressResult.SUCCESS;
        }
        return BackPressResult.FAILURE;
    }

    @Override
    public ObservableSupplier<Boolean> getHandleBackPressChangedSupplier() {
        return mHandleBackPressSupplier;
    }

    @Override
    public void selectTabAndHideHub(int tabId) {
        mHubLayoutController.selectTabAndHideHubLayout(tabId);
    }

    @Override
    public void focusPane(@PaneId int paneId) {
        mPaneManager.focusPane(paneId);
    }

    private @Nullable Pane getFocusedPane() {
        return mPaneManager.getFocusedPaneSupplier().get();
    }

    private boolean startSurfaceHandlesBackPress() {
        Tab currentTab = mCurrentTabSupplier.get();
        boolean isIncognito = currentTab != null ? currentTab.isIncognito() : false;
        return !isIncognito
                && START_SURFACE_LAYOUT_TYPE.equals(
                        mHubLayoutController.getPreviousLayoutTypeSupplier().get());
    }

    private void updateHandleBackPressSupplier() {
        boolean shouldHandleBackPress =
                Boolean.TRUE.equals(mFocusedPaneHandleBackPressSupplier.get())
                        || mPaneBackStackHandler.getHandleBackPressChangedSupplier().get()
                        || (!startSurfaceHandlesBackPress() && mCurrentTabSupplier.get() != null);
        mHandleBackPressSupplier.set(shouldHandleBackPress);
    }

    private <T> Callback<T> castCallback(Callback callback) {
        return (Callback<T>) callback;
    }

    private void setupViewForHubPaneHostView() {
        hubPaneHostView.setClipToOutline(true); 
        hubPaneHostView.setOutlineProvider(new ViewOutlineProvider() {
            @Override
            public void getOutline(View view, Outline outline) {
                float radius = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 24, view.getResources().getDisplayMetrics());
                outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), radius);

                outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), radius);
                outline.setConvexPath(createRoundedRectPath(view.getWidth(), view.getHeight(), radius));
            }
        });
    }

    private void setupViewForHubToolbarView() {
        hubToolbarView.setClipToOutline(true); 
        hubToolbarView.setOutlineProvider(new ViewOutlineProvider() {
            @Override
            public void getOutline(View view, Outline outline) {
                float radius = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 24, view.getResources().getDisplayMetrics());
                outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), radius);

                outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), radius);
                outline.setConvexPath(createRoundedRectPathForHubToolbarView(view.getWidth(), view.getHeight(), radius));
            }
        });
    }

    private android.graphics.Path createRoundedRectPath(int width, int height, float radius) {
        android.graphics.Path path = new android.graphics.Path();
        path.moveTo(0, radius); 
        path.lineTo(0, height); 
        path.lineTo(width, height); 
        path.lineTo(width, radius); 
        path.quadTo(width, 0, width - radius, 0); 
        path.lineTo(radius, 0);
        path.quadTo(0, 0, 0, radius);
        path.close();
        return path;
    }
    
    private android.graphics.Path createRoundedRectPathForHubToolbarView(int width, int height, float radius) {
        android.graphics.Path path = new android.graphics.Path();
        path.moveTo(0, height - radius);    
        path.lineTo(0, 0);
        path.lineTo(width, 0);
        path.lineTo(width, height-radius);
        path.quadTo(width, height, width - radius, height);
        path.lineTo(radius, height);
        path.quadTo(0, height, 0, height - radius);
        path.close();
        return path;
    }

    private void setHubBackgroundColor() {
        if(mCurrentTabSupplier == null) {
            return;
        }
        Tab currentTab = mCurrentTabSupplier.get();
        boolean isIncognito = currentTab != null ? currentTab.isIncognito() : false;

        if(mMainHubParent != null) {
            if(isIncognito) {
                mMainHubParent.setBackgroundColor(Color.parseColor("#000000"));
            } 
            else {
                mMainHubParent.setBackgroundColor(Color.parseColor("#FFFFFF"));
            }
        }
    } 

}