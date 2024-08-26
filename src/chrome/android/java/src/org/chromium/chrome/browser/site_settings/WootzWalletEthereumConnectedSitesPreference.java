/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.site_settings;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;

import androidx.preference.Preference;
import androidx.preference.PreferenceViewHolder;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.wootz_wallet.mojom.WootzWalletService;
import org.chromium.wootz_wallet.mojom.CoinType;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.wootz_wallet.WootzWalletServiceFactory;
import org.chromium.mojo.bindings.ConnectionErrorHandler;
import org.chromium.mojo.system.MojoException;

public class WootzWalletEthereumConnectedSitesPreference
        extends Preference implements ConnectionErrorHandler,
                                      WootzWalletEthereumConnectedSitesListAdapter
                                              .WootzEthereumPermissionConnectedSitesDelegate {
    private RecyclerView mRecyclerView;
    private WootzWalletService mWootzWalletService;
    private WootzWalletEthereumConnectedSitesListAdapter mAdapter;

    public WootzWalletEthereumConnectedSitesPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder holder) {
        super.onBindViewHolder(holder);

        initWootzWalletService();

        mRecyclerView = (RecyclerView) holder.findViewById(R.id.connected_sites_list);
        updateWebSitestList();
    }

    public void destroy() {
        mWootzWalletService.close();
        mWootzWalletService = null;
    }

    @SuppressLint("NotifyDataSetChanged")
    private void updateWebSitestList() {
        mWootzWalletService.getWebSitesWithPermission(CoinType.ETH, webSites -> {
            if (mAdapter == null) {
                mAdapter = new WootzWalletEthereumConnectedSitesListAdapter(webSites, this);
                mRecyclerView.setAdapter(mAdapter);
            } else {
                mAdapter.setWebSites(webSites);
                mAdapter.notifyDataSetChanged();
            }
        });
    }

    @Override
    public void removePermission(String webSite) {
        mWootzWalletService.resetWebSitePermission(CoinType.ETH, webSite, success -> {
            if (success) {
                updateWebSitestList();
            }
        });
    }

    @Override
    public void onConnectionError(MojoException e) {
        mWootzWalletService.close();
        mWootzWalletService = null;
        initWootzWalletService();
    }

    private void initWootzWalletService() {
        if (mWootzWalletService != null) {
            return;
        }

        mWootzWalletService = WootzWalletServiceFactory.getInstance().getWootzWalletService(this);
    }
}
