package org.chromium.chrome.browser.vpn.activities;

import android.graphics.Rect;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.Toolbar;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.vpn.WootzVpnNativeWorker;
import org.chromium.chrome.browser.vpn.adapters.WootzVpnServerSelectionAdapter;
import org.chromium.chrome.browser.vpn.models.WootzVpnPrefModel;
import org.chromium.chrome.browser.vpn.models.WootzVpnServerRegion;
import org.chromium.chrome.browser.vpn.utils.WootzVpnPrefUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnUtils;
import org.chromium.ui.widget.Toast;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class VpnServerSelectionActivity extends WootzVpnParentActivity {
    private WootzVpnServerSelectionAdapter mWootzVpnServerSelectionAdapter;
    private LinearLayout mServerSelectionListLayout;
    private ProgressBar mServerSelectionProgress;
    private RecyclerView mServerRegionList;

    public interface OnServerRegionSelection {
        void onServerRegionClick(WootzVpnServerRegion vpnServerRegion);
    }

    @Override
    public void onResumeWithNative() {
        super.onResumeWithNative();
        WootzVpnNativeWorker.getInstance().addObserver(this);
    }

    @Override
    public void onPauseWithNative() {
        WootzVpnNativeWorker.getInstance().removeObserver(this);
        super.onPauseWithNative();
    }

    private void initializeViews() {
        setContentView(R.layout.activity_vpn_server_selection);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        assert actionBar != null;
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setTitle(getResources().getString(R.string.change_location));

        TextView automaticText = (TextView) findViewById(R.id.automatic_server_text);
        automaticText.setText(getResources().getString(R.string.automatic));
        if (WootzVpnPrefUtils.getServerRegion()
                .equals(WootzVpnPrefUtils.PREF_WOOTZ_VPN_AUTOMATIC)) {
            automaticText.setCompoundDrawablesWithIntrinsicBounds(
                    0, 0, R.drawable.ic_checkbox_checked, 0);
        } else {
            automaticText.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, 0);
        }
        automaticText.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        changeServerRegion(
                                new WootzVpnServerRegion(
                                        "", "", WootzVpnPrefUtils.PREF_WOOTZ_VPN_AUTOMATIC, ""));
                    }
                });

        mServerSelectionListLayout = (LinearLayout) findViewById(R.id.server_selection_list_layout);
        mServerSelectionProgress = (ProgressBar) findViewById(R.id.server_selection_progress);

        mServerRegionList = (RecyclerView) findViewById(R.id.server_selection_list);
        LinearLayoutManager linearLayoutManager = new LinearLayoutManager(this);
        mServerRegionList.addItemDecoration(
                new DividerItemDecoration(
                        VpnServerSelectionActivity.this, linearLayoutManager.getOrientation()) {
                    @Override
                    public void getItemOffsets(
                            Rect outRect,
                            View view,
                            RecyclerView parent,
                            RecyclerView.State state) {
                        int position = parent.getChildAdapterPosition(view);
                        // hide the divider for the last child
                        if (position == state.getItemCount() - 1) {
                            outRect.setEmpty();
                        } else {
                            super.getItemOffsets(outRect, view, parent, state);
                        }
                    }
                });
        mServerRegionList.setLayoutManager(linearLayoutManager);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void finishNativeInitialization() {
        super.finishNativeInitialization();
        WootzVpnNativeWorker.getInstance().getAllServerRegions();
        showProgress();
    }

    @Override
    public void onGetAllServerRegions(String jsonResponse, boolean isSuccess) {
        if (isSuccess) {
            List<WootzVpnServerRegion> wootzVpnServerRegions =
                    WootzVpnUtils.getServerLocations(jsonResponse);
            Collections.sort(
                    wootzVpnServerRegions,
                    new Comparator<WootzVpnServerRegion>() {
                        @Override
                        public int compare(
                                WootzVpnServerRegion wootzVpnServerRegion1,
                                WootzVpnServerRegion wootzVpnServerRegion2) {
                            return wootzVpnServerRegion1
                                    .getNamePretty()
                                    .compareToIgnoreCase(wootzVpnServerRegion2.getNamePretty());
                        }
                    });
            mWootzVpnServerSelectionAdapter = new WootzVpnServerSelectionAdapter();
            mWootzVpnServerSelectionAdapter.setVpnServerRegions(wootzVpnServerRegions);
            mWootzVpnServerSelectionAdapter.setOnServerRegionSelection(
                    new OnServerRegionSelection() {
                        @Override
                        public void onServerRegionClick(WootzVpnServerRegion wootzVpnServerRegion) {
                            if (WootzVpnPrefUtils.getServerRegion()
                                    .equals(wootzVpnServerRegion.getName())) {
                                Toast.makeText(
                                                VpnServerSelectionActivity.this,
                                                R.string.already_selected_the_server,
                                                Toast.LENGTH_SHORT)
                                        .show();
                            } else {
                                changeServerRegion(wootzVpnServerRegion);
                            }
                        }
                    });
            mServerRegionList.setAdapter(mWootzVpnServerSelectionAdapter);
            hideProgress();
        } else {
            Toast.makeText(
                            VpnServerSelectionActivity.this,
                            R.string.fail_to_get_server_locations,
                            Toast.LENGTH_LONG)
                    .show();
        }
    }

    @Override
    protected void triggerLayoutInflation() {
        initializeViews();
        onInitialLayoutInflationComplete();
    }

    @Override
    public boolean shouldStartGpuProcess() {
        return true;
    }

    private void changeServerRegion(WootzVpnServerRegion wootzVpnServerRegion) {
        mIsServerLocationChanged = true;
        WootzVpnUtils.selectedServerRegion = wootzVpnServerRegion;
        WootzVpnUtils.showProgressDialog(
                VpnServerSelectionActivity.this,
                getResources().getString(R.string.vpn_connect_text));
    }

    public void showProgress() {
        if (mServerSelectionProgress != null) {
            mServerSelectionProgress.setVisibility(View.VISIBLE);
        }
        if (mServerSelectionListLayout != null) {
            mServerSelectionListLayout.setAlpha(0.4f);
        }
    }

    public void hideProgress() {
        if (mServerSelectionProgress != null) {
            mServerSelectionProgress.setVisibility(View.GONE);
        }
        if (mServerSelectionListLayout != null) {
            mServerSelectionListLayout.setAlpha(1f);
        }
    }

    @Override
    public void showRestoreMenu(boolean shouldShowRestore) {}

    @Override
    public void updateProfileView() {}
}