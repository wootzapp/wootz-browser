package org.chromium.chrome.browser.vpn.activities;

import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.widget.Toolbar;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.vpn.WootzVpnNativeWorker;
import org.chromium.chrome.browser.vpn.models.WootzVpnPrefModel;
import org.chromium.chrome.browser.vpn.utils.WootzVpnUtils;

public class WootzVpnProfileActivity extends WootzVpnParentActivity {
    private TextView mProfileTitle;
    private TextView mProfileText;
    private Button mInstallVpnButton;
    private Button mContactSupportButton;
    private ProgressBar mProfileProgress;
    private LinearLayout mProfileLayout;

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
        setContentView(R.layout.activity_wootz_vpn_profile);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        ActionBar actionBar = getSupportActionBar();
        assert actionBar != null;
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(R.drawable.ic_baseline_close_24);
        actionBar.setTitle(getResources().getString(R.string.install_vpn));

        mProfileProgress = findViewById(R.id.profile_progress);
        mProfileLayout = findViewById(R.id.profile_layout);

        mProfileTitle = findViewById(R.id.wootz_vpn_profile_title);
        mProfileText = findViewById(R.id.wootz_vpn_profile_text);

        mInstallVpnButton = findViewById(R.id.btn_install_profile);
        mInstallVpnButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                WootzVpnUtils.showProgressDialog(WootzVpnProfileActivity.this,
                        getResources().getString(R.string.vpn_connect_text));
                
                mWootzVpnPrefModel = new WootzVpnPrefModel();
                
            }
        });

        mContactSupportButton = findViewById(R.id.btn_contact_supoort);
        mContactSupportButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                WootzVpnUtils.openWootzVpnSupportActivity(WootzVpnProfileActivity.this);
            }
        });
    }

    @Override
    public void finishNativeInitialization() {
        super.finishNativeInitialization();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
        }
        return super.onOptionsItemSelected(item);
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

    @Override
    public void showRestoreMenu(boolean shouldShowRestore) {}

    @Override
    public void updateProfileView() {
        mProfileTitle.setText(getResources().getString(R.string.some_context));
        mProfileText.setText(getResources().getString(R.string.some_context_text));
        mInstallVpnButton.setText(getResources().getString(R.string.accept_connection_request));
        mContactSupportButton.setVisibility(View.GONE);
    }
}