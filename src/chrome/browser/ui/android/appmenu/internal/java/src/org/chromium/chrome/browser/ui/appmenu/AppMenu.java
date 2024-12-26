// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ui.appmenu;

import android.animation.Animator;
import android.animation.AnimatorSet;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.os.SystemClock;
import android.os.Handler;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.View;
import android.view.View.MeasureSpec;
import android.view.View.OnKeyListener;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.ViewStub;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.os.Build;
import android.widget.GridView;
import androidx.annotation.NonNull;
import android.app.Dialog;
import com.google.android.material.bottomsheet.BottomSheetDialog;
import com.google.android.material.bottomsheet.BottomSheetBehavior;
import android.view.ViewGroup.LayoutParams;
import com.google.android.material.bottomsheet.BottomSheetDialogFragment;
import android.os.Bundle;
import androidx.annotation.ColorInt;
import androidx.annotation.IdRes;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import org.chromium.chrome.browser.extensions.ExtensionInfo;
import org.chromium.chrome.browser.extensions.Extensions;
import org.chromium.base.ContextUtils;
import org.chromium.components.embedder_support.view.ContentView;
import org.chromium.components.thinwebview.ThinWebView;
import org.chromium.components.thinwebview.ThinWebViewConstraints;
import org.chromium.components.thinwebview.ThinWebViewFactory;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.browser.WebContentsObserver;
import org.chromium.chrome.browser.content.WebContentsFactory;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.profiles.ProfileManager;
import org.chromium.base.version_info.VersionInfo;
import org.chromium.ui.base.IntentRequestTracker;
import org.chromium.ui.base.ViewAndroidDelegate;
import org.chromium.base.Callback;
import org.chromium.base.SysUtils;
import org.chromium.base.metrics.RecordHistogram;
import org.chromium.base.task.PostTask;
import org.chromium.base.task.TaskTraits;
import org.chromium.chrome.browser.ui.appmenu.internal.R;
import org.chromium.components.browser_ui.styles.ChromeColors;
import org.chromium.components.browser_ui.widget.chips.ChipView;
import org.chromium.components.browser_ui.widget.highlight.ViewHighlighter;
import org.chromium.components.browser_ui.widget.highlight.ViewHighlighter.HighlightParams;
import org.chromium.components.browser_ui.widget.highlight.ViewHighlighter.HighlightShape;
import org.chromium.ui.modelutil.MVCListAdapter.ModelList;
import org.chromium.ui.modelutil.ModelListAdapter;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.ui.widget.Toast;
import android.widget.BaseAdapter;

import java.beans.Visibility;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import android.widget.ImageView;
import android.widget.TextView;
import androidx.fragment.app.FragmentManager;
import android.util.Log;
import androidx.core.graphics.drawable.DrawableCompat;
import android.database.DataSetObserver;

import androidx.core.widget.NestedScrollView;
import android.widget.LinearLayout;
import androidx.coordinatorlayout.widget.CoordinatorLayout;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.graphics.Outline;
import android.view.ViewOutlineProvider;
import android.os.Build;
import android.widget.FrameLayout;
import android.widget.HorizontalScrollView;
import org.chromium.base.task.PostTask;
import org.chromium.base.task.TaskTraits;
import org.chromium.chrome.browser.tab.Tab;
import androidx.appcompat.content.res.AppCompatResources;

/**
 * Shows a popup of menuitems anchored to a host view. When a item is selected we call
 * AppMenuHandlerImpl.AppMenuDelegate.onOptionsItemSelected with the appropriate MenuItem.
 *   - Only visible MenuItems are shown.
 *   - Disabled items are grayed out.
 */
/*
 * This class has been revamped by Devendra(dkt) by adding BottomSheetDialogFragment
 * as the parent class of AppMenu which enables the use of BottomSheet directly when
 * Menu button is pressed while also implementing the exisiting interfaces.
 */
public class AppMenu extends BottomSheetDialogFragment implements OnItemClickListener, OnKeyListener, AppMenuClickHandler {
    private static final String TAG = "AppMenu";
    private static final float LAST_ITEM_SHOW_FRACTION = 0.5f;

    /** A means of reporting an exception/stack without crashing. */
    private static Callback<Throwable> sExceptionReporter;

    private int mItemRowHeight;
    private int mVerticalFadeDistance;
    private int mNegativeSoftwareVerticalOffset;
    private int mNegativeVerticalOffsetNotTopAnchored;
    private int mChipHighlightExtension;
    private int[] mTempLocation;

    private GridView mGridView;
    private static final int GRID_COLUMNS = 3; // Adjust as needed

    private ModelListAdapter mAdapter;
    private AppMenuHandlerImpl mHandler;
    private int mCurrentScreenRotation = -1;
    private boolean mIsByPermanentButton;
    private long mMenuShownTimeMs;
    private boolean mSelectedItemBeforeDismiss;
    private ModelList mModelList;

    private int mHeaderResourceId;
    private int mFooterResourceId;

    private GridAdapter mGridAdapter;
    private NestedScrollView mScrollView;
    private BottomSheetBehavior<View> mBehavior;
    private ImageButton mFloatingBackButton;
    private WebContents mCurrentWebContents;
    private WebContents mWebContents;
    private ContentView mContentView;
    private ThinWebView mThinWebView;
    private View mWebViewContainer;

    /**
     * Creates and sets up the App Menu.
     * @param itemRowHeight Desired height for each app menu row.
     * @param handler AppMenuHandlerImpl receives callbacks from AppMenu.
     * @param res Resources object used to get dimensions and style attributes.
     */
    // By Devendra(dkt)
    //Required empty constructor
    public AppMenu() {
        mVerticalFadeDistance = 0;
        mNegativeSoftwareVerticalOffset = 0;
        mNegativeVerticalOffsetNotTopAnchored = 0;
        mChipHighlightExtension = 0;
        mTempLocation = new int[2];
        mItemRowHeight = 0;
        mHeaderResourceId = 0;
        mFooterResourceId = 0;
    }

    public static AppMenu newInstance(int itemRowHeight, AppMenuHandlerImpl handler, Resources res) {
        AppMenu fragment = new AppMenu();
        fragment.mItemRowHeight = itemRowHeight;
        fragment.mHandler = handler;
        fragment.initializeWithResources(res);
        return fragment;
    }

    private void initializeWithResources(Resources res) {
        mVerticalFadeDistance = res.getDimensionPixelSize(R.dimen.menu_vertical_fade_distance);
        mNegativeSoftwareVerticalOffset = res.getDimensionPixelSize(R.dimen.menu_negative_software_vertical_offset);
        mNegativeVerticalOffsetNotTopAnchored = res.getDimensionPixelSize(R.dimen.menu_negative_vertical_offset_not_top_anchored);
        mChipHighlightExtension = res.getDimensionPixelOffset(R.dimen.menu_chip_highlight_extension);
    }

    /**
     * Notifies the menu that the contents of the menu item specified by {@code menuRowId} have
     * changed.  This should be called if icons, titles, etc. are changing for a particular menu
     * item while the menu is open.
     * @param menuRowId The id of the menu item to change.  This must be a row id and not a child
     *                  id.crollView = new NestedScrollView(getContext());
     */
    public void menuItemContentChanged(int menuRowId) {
        // Make sure we have all the valid state objects we need.
        if (mAdapter == null || mModelList == null || mGridView == null) {
            return;
        }

        // Calculate the item index.
        int index = -1;
        int menuSize = mModelList.size();
        for (int i = 0; i < menuSize; i++) {
            if (mModelList.get(i).model.get(AppMenuItemProperties.MENU_ITEM_ID) == menuRowId) {
                index = i;
                break;
            }
        }
        if (index == -1) return;

        // Check if the item is visible.
        int startIndex = mGridView.getFirstVisiblePosition();
        int endIndex = mGridView.getLastVisiblePosition();
        if (index < startIndex || index > endIndex) return;

        // Grab the correct View.
        View view = mGridView.getChildAt(index - startIndex);
        if (view == null) return;

        // Cause the Adapter to re-populate the View.
        mAdapter.getView(index, view, mGridView);
    }

    @NonNull
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        BottomSheetDialog dialog = (BottomSheetDialog) super.onCreateDialog(savedInstanceState);
        View view = createContentView(true);
        dialog.setContentView(view);

        // Log.d(TAG,"mBottomSheet : " + view.getParent().toString()); 

        // this code removes the dark scrim behind the menu
        Window window = dialog.getWindow();
        if (window != null) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        } 

        mBehavior = BottomSheetBehavior.from((View) view.getParent());
        mBehavior.setSkipCollapsed(true);
        mBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);

        // Disable dragging on the BottomSheetBehavior
        mBehavior.setDraggable(false);/*pass true for draggable behaviour */

        return dialog;
    }

    private View createContentView(boolean test) {
        NestedScrollView scrollView = new NestedScrollView(getContext());
        scrollView.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
        
        // Create a FrameLayout to wrap the GridView
        FrameLayout gridViewWrapper = new FrameLayout(getContext());
        FrameLayout.LayoutParams wrapperParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        
        // Set margins for the wrapper (adjust these values as needed)
        int margin = dpToPx(32); // Convert 16dp to pixels
        wrapperParams.setMargins(
            0,
            -margin, 
            0, 
            margin
        );
        gridViewWrapper.setLayoutParams(wrapperParams);
        
        return scrollView;
    }

    private View createWebViewContainer() {
        FrameLayout viewWrapper = new FrameLayout(getContext());
        FrameLayout.LayoutParams wrapperParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        
        // Set horizontal margins to create padding from screen edges
        int horizontalMargin = dpToPx(0);
        int verticalMargin = dpToPx(0);
        wrapperParams.setMargins(horizontalMargin, verticalMargin, horizontalMargin, verticalMargin);
        viewWrapper.setLayoutParams(wrapperParams);
    
        // Initialize WebView components only once
        if (mWebContents == null) {
            Profile profile = ProfileManager.getLastUsedRegularProfile();
            mWebContents = WebContentsFactory.createWebContents(profile, true, false);
            mContentView = ContentView.createContentView(getContext(), null, mWebContents);
            mWebContents.setDelegates(
                VersionInfo.getProductVersion(),
                ViewAndroidDelegate.createBasicDelegate(mContentView),
                mContentView,
                mHandler.getWindowAndroid(),
                WebContents.createDefaultInternalsHolder());
    
            IntentRequestTracker intentRequestTracker = mHandler.getWindowAndroid().getIntentRequestTracker();
            mThinWebView = ThinWebViewFactory.create(
                getContext(), new ThinWebViewConstraints(), intentRequestTracker);
            mThinWebView.attachWebContents(mWebContents, mContentView, null);
            
            // Apply corner radius to the ThinWebView
            View webView = mThinWebView.getView();
            float cornerRadius = dpToPx(16); // Adjust this value for desired roundness
            webView.setClipToOutline(true);
            webView.setOutlineProvider(new ViewOutlineProvider() {
                @Override
                public void getOutline(View view, Outline outline) {
                    outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), cornerRadius);
                }
            });

            // Set minimal padding on the WebView itself
            webView.setPadding(0, 0, 0, 0);
        
            // Ensure the WebView fills its container
            webView.setLayoutParams(new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT));

            // Set background color to match the design
            // webView.setBackgroundColor(Color.parseColor("#1C1E21")); // Dark background color
        }
    
        viewWrapper.addView(mThinWebView.getView());
        
        // Apply same corner radius to the wrapper for consistency
        float wrapperCornerRadius = dpToPx(16);
        viewWrapper.setClipToOutline(true);
        viewWrapper.setOutlineProvider(new ViewOutlineProvider() {
            @Override
            public void getOutline(View view, Outline outline) {
                outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), wrapperCornerRadius);
            }
        });
        
        // Set background color to wrapper as well
        // viewWrapper.setBackgroundColor(Color.parseColor("#1C1E21"));
    
        return viewWrapper;
    }

    private void returnToAppMenu() {
        View view = getView();
        if (view != null) {
            view.findViewById(R.id.app_menu_grid).setVisibility(View.VISIBLE);
            view.findViewById(R.id.app_menu_extensions).setVisibility(View.VISIBLE);
            view.findViewById(R.id.extensions_divider).setVisibility(View.VISIBLE);
            view.findViewById(R.id.web_view_container).setVisibility(View.GONE);
            if (mFloatingBackButton != null) {
                mFloatingBackButton.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        
        View parent = (View) getView().getParent();
        parent.setBackgroundColor(Color.TRANSPARENT);

        CoordinatorLayout.LayoutParams layoutParams = (CoordinatorLayout.LayoutParams) parent.getLayoutParams();
        
        // Using hardcoded values: 16dp for left, right, and bottom margins
        int marginInPixels = dpToPx(16);
        
        layoutParams.setMargins(
            32,  // LEFT
            -32,               // TOP set the margin here
            32,  // RIGHT
            32   // BOTTOM /* for some reson this doesn't work so set negative margin on top */
        );
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            float borderRadius = dpToPx(24); // You can adjust this value as needed
            parent.setClipToOutline(true);
            parent.setOutlineProvider(new ViewOutlineProvider() {
                @Override
                public void getOutline(View view, Outline outline) {
                    outline.setRoundRect(0, 0, view.getWidth(), view.getHeight(), borderRadius);
                }
            });
        }
        parent.setLayoutParams(layoutParams);

        View gridView = parent.findViewById(R.id.app_menu_grid);
        gridView.setVisibility(View.VISIBLE);
        createExtensionsRow();
        FrameLayout webView = (FrameLayout) parent.findViewById(R.id.web_view);
        webView.setVisibility(View.GONE);

        // webView.addView(createWebView());


        // Set up bottom sheet callback to maintain bottom margin when expanded
        BottomSheetBehavior<View> behavior = BottomSheetBehavior.from(parent);
        behavior.addBottomSheetCallback(new BottomSheetBehavior.BottomSheetCallback() {
            @Override
            public void onStateChanged(@NonNull View bottomSheet, int newState) {
                if (newState == BottomSheetBehavior.STATE_EXPANDED) {
                    bottomSheet.setPadding(0, 0, 0, marginInPixels);
                } else {
                    bottomSheet.setPadding(0, 0, 0, 0);
                }
            }

            @Override
            public void onSlide(@NonNull View bottomSheet, float slideOffset) {
                // Not needed for this implementation
            }
        });
    }

    private int dpToPx(int dp) {
        float density = getResources().getDisplayMetrics().density;
        return Math.round((float) dp * density);
    }

    private void createExtensionsRow() {
        Context context = getContext();
        View view = getView();
        if(view == null) return;

        View extensionsDivider = view.findViewById(R.id.extensions_divider);
        LinearLayout extensionsContainer = view.findViewById(R.id.app_menu_extensions_container);
        HorizontalScrollView scrollView = view.findViewById(R.id.extensions_scroll_view);
        LinearLayout parent = view.findViewById(R.id.app_menu_extensions);

        extensionsContainer.removeAllViews();

        List<ExtensionInfo> extensionsInfo = Extensions.getExtensionsInfo();
        int extensionCount = extensionsInfo.size();

        extensionsDivider.setVisibility(View.VISIBLE);
        scrollView.setVisibility(View.VISIBLE);
        parent.setVisibility(View.VISIBLE);

        int buttonSize = dpToPx(48);
        int buttonMargin = dpToPx(4);
        int containerWidth = buttonSize * 5 + buttonMargin * 10; // Adjusted for new margins

        // LinearLayout.LayoutParams containerParams = new LinearLayout.LayoutParams(
        //         ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        // extensionsContainer.setLayoutParams(containerParams);

        // Add "Add Extension" button
        ImageButton addExtensionButton = createRoundButton(context);
        addExtensionButton.setImageResource(R.drawable.ic_add_extensions);
        addExtensionButton.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        addExtensionButton.setImageTintList(AppCompatResources.getColorStateList(context, R.color.extension_icon_color));
        addExtensionButton.setOnClickListener(v -> openWebsite("https://github.com/wootzapp/ext-store"));
        extensionsContainer.addView(addExtensionButton);

        for (int i = 0; i < extensionCount; i++) {
            ExtensionInfo extension = extensionsInfo.get(i);
            ImageButton extensionIcon = createRoundButton(context);
            if (extension.getName().equals("Web Store")) {
                continue;
            }
            if (extension.getIconBitmap() != null) {
                extensionIcon.setImageBitmap(extension.getIconBitmap());
                extensionIcon.setImageTintList(null); // Remove any tint
            } else {
                extensionIcon.setImageResource(R.drawable.test_extension_logo);
                extensionIcon.setImageTintList(AppCompatResources.getColorStateList(context, R.color.extension_icon_color));
            }

            final int index = i;
            extensionIcon.setOnClickListener(v -> openExtensionWebView(index));
            extensionIcon.setOnLongClickListener(v -> {
                showDeleteExtensionDialog(index);
                return true;
            });

            extensionsContainer.addView(extensionIcon);
        }

        // Set the width of the scroll view to show only 5 buttons
        ViewGroup.LayoutParams scrollParams = scrollView.getLayoutParams();
        scrollParams.width = ViewGroup.LayoutParams.MATCH_PARENT;
        scrollView.setLayoutParams(scrollParams);

        // Enable horizontal scrolling
        scrollView.setHorizontalScrollBarEnabled(true);
    }

    private ImageButton createRoundButton(Context context) {
        ImageButton button = new ImageButton(context);
        int size = dpToPx(48); // Slightly smaller size
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(size, size);
        params.setMargins(dpToPx(8), 0, dpToPx(8), 0); // Reduced margins
        button.setLayoutParams(params);
    
        button.setBackground(AppCompatResources.getDrawable(context, R.drawable.extension_button_background));
        button.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        button.setPadding(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
    
        return button;
    }

    public void showExtensionWebViewDirectly(String extensionId, AppMenuExtensionOpener extensionOpener) {
        extensionOpener.openExtension(extensionId);
    }
    
    public void closeExtensionBottomSheet(AppMenuExtensionOpener extensionOpener) {
        extensionOpener.closeBottomSheet();
    }

    private void openWebsite(String url) {
        if (mHandler != null) {
            LoadUrlParams params = new LoadUrlParams(url);
            Tab tab = mHandler.getActivityTab();
            if (tab != null) {
                tab.loadUrl(params);
                dismiss(); // Dismiss the app menu after loading the URL
            }
        }
    }

    private void showDeleteExtensionDialog(int extensionIndex) {
        Context context = getContext();
        if (context == null) return;
    
        new androidx.appcompat.app.AlertDialog.Builder(context)
            .setTitle("Delete Extension")
            .setMessage("Do you want to delete this extension?")
            .setPositiveButton("Delete", (dialog, which) -> {
                deleteExtension(extensionIndex);
            })
            .setNegativeButton("Cancel", null)
            .show();
    }

    private void deleteExtension(int extensionIndex) {
        // Extensions.getExtensionsInfo().remove(extensionIndex);
        String extensionId = Extensions.getExtensionsInfo().get(extensionIndex).getId();
        Log.d(TAG,"Deleting extension " + extensionId);
        Extensions.uninstallExtension(extensionId);
        createExtensionsRow();
    }

    private void openExtensionWebView(int index) {
        View view = getView();
        if (view != null) {
            view.findViewById(R.id.app_menu_grid).setVisibility(View.GONE);
            view.findViewById(R.id.app_menu_extensions).setVisibility(View.GONE);
            view.findViewById(R.id.extensions_divider).setVisibility(View.GONE);
            
            FrameLayout webViewContainer = view.findViewById(R.id.web_view_container);
            webViewContainer.setVisibility(View.VISIBLE);
            
            FrameLayout webViewFrame = view.findViewById(R.id.web_view);
            webViewFrame.removeAllViews();
            webViewFrame.setVisibility(View.VISIBLE);
            
            if (mWebViewContainer == null) {
                mWebViewContainer = createWebViewContainer();
            }
            webViewFrame.addView(mWebViewContainer);
    
            // Load the extension URL
            String popupUrl = Extensions.getExtensionsInfo().get(index).getPopupUrl();
            mWebContents.getNavigationController().loadUrl(new LoadUrlParams(popupUrl));
        }
    }
    
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if (mWebContents != null) {
            mWebContents.destroy();
            mWebContents = null;
        }
        mContentView = null;
        mThinWebView = null;
        mWebViewContainer = null;
    }

    public boolean onBackPressed() {
        View view = getView();
        if (view != null && view.findViewById(R.id.app_menu_grid).getVisibility() == View.GONE) {
            returnToAppMenu();
            return true;
        }
        return false;
    }

    @Override
    public void show(@NonNull FragmentManager manager, @Nullable String tag) {
        Log.d(TAG, Extensions.getExtensionsInfo().toString());

        Log.d(TAG, "show called with tag: " + tag);
        try {
            super.show(manager, tag);
            mMenuShownTimeMs = SystemClock.elapsedRealtime();
            if (mHandler != null) {
                mHandler.onMenuVisibilityChanged(true);
            }
            Log.d(TAG, "AppMenu shown successfully");
        } catch (Exception e) {
            Log.e(TAG, "Error showing AppMenu", e);
        }
    }

    @Override
    public void dismiss() {
        Log.d(TAG, "dismiss called");
        try {
            super.dismiss();
            recordTimeToTakeActionHistogram();
            if (mHandler != null) {
                mHandler.appMenuDismissed();
                mHandler.onMenuVisibilityChanged(false);
            }
            Log.d(TAG, "AppMenu dismissed successfully");
        } catch (Exception e) {
            Log.e(TAG, "Error dismissing AppMenu", e);
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        int originalPosition = (int) id;
        PropertyModel model = mModelList.get(originalPosition).model;
        if (model != null && model.get(AppMenuItemProperties.ENABLED)) {
            onItemClick(model);
        }
    }

    @Override
    public void onItemClick(PropertyModel model) {
        if (!model.get(AppMenuItemProperties.ENABLED)) return;

        int id = model.get(AppMenuItemProperties.MENU_ITEM_ID);
        mSelectedItemBeforeDismiss = true;
        // dismiss();f
        if (mHandler != null) {
            Log.d(TAG,"Entering the onItemClick for " + id);
            mHandler.onOptionsItemSelected(id);
            Log.d(TAG,"Exiting the onItemClick for " + id);
        }
        // this dismiss is added after the option is selected
        dismiss();
    }

    @Override
    public boolean onItemLongClick(PropertyModel model, View view) {
        Log.d(TAG,"Enterning the onItemLongClick " + view);
        if (!model.get(AppMenuItemProperties.ENABLED)) return false;
        Log.d(TAG,"After the conditional check in onItemLongClick");
        mSelectedItemBeforeDismiss = true;
        CharSequence titleCondensed = model.get(AppMenuItemProperties.TITLE_CONDENSED);
        CharSequence message =
                TextUtils.isEmpty(titleCondensed)
                        ? model.get(AppMenuItemProperties.TITLE)
                        : titleCondensed;
        return showToastForItem(message, view);
    }

    @VisibleForTesting
    boolean showToastForItem(CharSequence message, View view) {
        Context context = view.getContext();
        final @ColorInt int backgroundColor =
                ChromeColors.getSurfaceColor(context, R.dimen.toast_elevation);
        return new Toast.Builder(context)
                .withText(message)
                .withAnchoredView(view)
                .withBackgroundColor(backgroundColor)
                .withTextAppearance(R.style.TextAppearance_TextSmall_Primary)
                .buildAndShow();
    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        if (mGridView == null) return false;
        if (event.getKeyCode() == KeyEvent.KEYCODE_MENU) {
            if (event.getAction() == KeyEvent.ACTION_DOWN && event.getRepeatCount() == 0) {
                event.startTracking();
                v.getKeyDispatcherState().startTracking(event, this);
                return true;
            } else if (event.getAction() == KeyEvent.ACTION_UP) {
                v.getKeyDispatcherState().handleUpEvent(event);
                if (event.isTracking() && !event.isCanceled()) {
                    dismiss();
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Update the menu items.
     * @param newModelList The new menu item list will be displayed.
     * @param adapter The adapter for visible items in the Menu.
     */
    void updateMenu(ModelList newModelList, ModelListAdapter adapter) {
        mModelList = newModelList;
        mAdapter = adapter;
        if (mGridAdapter != null) {
            mGridAdapter.updateValidItems();
        }
    }

    /**
     * Find the {@link PropertyModel} associated with the given id. If the menu item is not found,
     * return null.
     * @param itemId The id of the menu item to find.
     * @return The {@link PropertyModel} has the given id. null if not found.
     */
    PropertyModel getMenuItemPropertyModel(int itemId) {
        for (int i = 0; i < mModelList.size(); i++) {
            PropertyModel model = mModelList.get(i).model;
            if (model.get(AppMenuItemProperties.MENU_ITEM_ID) == itemId) {
                return model;
            } else if (model.get(AppMenuItemProperties.SUBMENU) != null) {
                ModelList subList = model.get(AppMenuItemProperties.SUBMENU);
                for (int j = 0; j < subList.size(); j++) {
                    PropertyModel subModel = subList.get(j).model;
                    if (subModel.get(AppMenuItemProperties.MENU_ITEM_ID) == itemId) {
                        return subModel;
                    }
                }
            }
        }
        return null;
    }

    /** Invalidate the app menu data. See {@link AppMenuAdapter#notifyDataSetChanged}. */
    void invalidate() {
        if (mAdapter != null) mAdapter.notifyDataSetChanged();
    }

    private void inflateHeader(int headerResourceId, View contentView) {
        if (headerResourceId == 0) return;

        View headerView = LayoutInflater.from(getContext()).inflate(headerResourceId, null);
        ViewGroup headerContainer = contentView.findViewById(R.id.app_menu_header);
        headerContainer.addView(headerView);

        if (mHandler != null) mHandler.onHeaderViewInflated(headerView);
    }

    private void inflateFooter(int footerResourceId, View contentView) {
        if (footerResourceId == 0) return;

        View footerView = LayoutInflater.from(getContext()).inflate(footerResourceId, null);
        ViewGroup footerContainer = contentView.findViewById(R.id.app_menu_footer);
        footerContainer.addView(footerView);

        if (mHandler != null) mHandler.onFooterViewInflated(footerView);
    }

    private void recordTimeToTakeActionHistogram() {
        final String histogramName =
                "Mobile.AppMenu.TimeToTakeAction."
                        + (mSelectedItemBeforeDismiss ? "SelectedItem" : "Abandoned");
        final long timeToTakeActionMs = SystemClock.elapsedRealtime() - mMenuShownTimeMs;
        RecordHistogram.recordMediumTimesHistogram(histogramName, timeToTakeActionMs);
    }

    private int getMenuItemHeight(
            int itemId, Context context, @Nullable List<CustomViewBinder> customViewBinders) {
        // Check if |item| is custom type
        if (customViewBinders != null) {
            for (int i = 0; i < customViewBinders.size(); i++) {
                CustomViewBinder binder = customViewBinders.get(i);
                if (binder.getItemViewType(itemId) != CustomViewBinder.NOT_HANDLED) {
                    return binder.getPixelHeight(context);
                }
            }
        }
        return mItemRowHeight;
    }

    /** @param reporter A means of reporting an exception without crashing. */
    static void setExceptionReporter(Callback<Throwable> reporter) {
        sExceptionReporter = reporter;
    }

    public void setHeaderResourceId(int headerResourceId) {
        mHeaderResourceId = headerResourceId;
    }

    public void setFooterResourceId(int footerResourceId) {
        mFooterResourceId = footerResourceId;
    }

    public boolean isShowing() {
        return getDialog() != null && getDialog().isShowing();
    }

    public GridView getGridView() {
        return mGridView;
    }

    private class GridAdapter extends BaseAdapter {
        private ModelList mModelList;
        private LayoutInflater mInflater;
        private Map<Integer, Integer> mDisplayToOriginalPosition;
        private List<Integer> mValidItemPositions;

        public GridAdapter(Context context, ModelList modelList) {
            mModelList = modelList;
            mInflater = LayoutInflater.from(context);
            updateValidItems();
        }

        public void updateValidItems() {
            if (mModelList == null) {
                return;
            }

            mDisplayToOriginalPosition = new HashMap<>();
            mValidItemPositions = new ArrayList<>();
            for (int i = 0; i < mModelList.size(); i++) {
                PropertyModel model = mModelList.get(i).model;
                if (isValidMenuItem(model)) {
                    mDisplayToOriginalPosition.put(mValidItemPositions.size(), i);
                    mValidItemPositions.add(i);
                }
            }
            notifyDataSetChanged();
        }

        private boolean isValidMenuItem(PropertyModel model) {
            return model != null &&
                   !TextUtils.isEmpty(model.get(AppMenuItemProperties.TITLE)) &&
                   model.get(AppMenuItemProperties.ICON) != null;
        }

        @Override
        public int getCount() {
            return mValidItemPositions.size();
        }

        @Override
        public Object getItem(int position) {
            int originalPosition = mValidItemPositions.get(position);
            return mModelList.get(originalPosition).model;
        }

        @Override
        public long getItemId(int position) {
            return mValidItemPositions.get(position);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.grid_item_layout, parent, false);
                convertView.setTag(new ViewHolder(convertView));
            }

            ViewHolder holder = (ViewHolder) convertView.getTag();
            PropertyModel model = (PropertyModel) getItem(position);
            holder.bindModel(model,convertView);

            return convertView;
        }

        private class ViewHolder {
            ImageView iconView;
            TextView titleView;

            ViewHolder(View view) {
                iconView = view.findViewById(R.id.item_icon);
                titleView = view.findViewById(R.id.item_title);
            }

            void bindModel(PropertyModel model,View view) {
                Drawable icon = model.get(AppMenuItemProperties.ICON);
                CharSequence title = model.get(AppMenuItemProperties.TITLE);

                if (icon != null) {
                    Drawable adaptiveIcon = DrawableCompat.wrap(icon.mutate());
                    DrawableCompat.setTint(adaptiveIcon, titleView.getCurrentTextColor());
                    iconView.setImageDrawable(adaptiveIcon);
                } else {
                    iconView.setImageDrawable(null);
                }

                if (title != null) {
                    title = title.toString()
                            .substring(0, 1)
                            .toUpperCase() + title.toString().substring(1);
                }
            
                titleView.setText(title);
            
                boolean isEnabled = model.get(AppMenuItemProperties.ENABLED);
                view.setEnabled(isEnabled);
                view.setAlpha(isEnabled ? 1.0f : 0.5f);
            }
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.app_menu_bottom_sheet_layout, container, false);
        
        LinearLayout contentLayout = view.findViewById(R.id.app_menu_content);
        
        // Add extensions row
        createExtensionsRow();
        // contentLayout.addView(createExtensionsRow(), 0);  // Add at the top

        mGridView = view.findViewById(R.id.app_menu_grid);
        mGridView.setNumColumns(GRID_COLUMNS);

        if (mModelList == null) {
            return view;
        }
        
        mGridAdapter = new GridAdapter(getContext(), mModelList);
        mGridView.setAdapter(mGridAdapter);
        
        mGridView.setOnItemClickListener(this);

        // ImageButton backButton = view.findViewById(R.id.back_to_menu_button);
        // backButton.setOnClickListener(v -> returnToAppMenu());

        return view;
    }
}
