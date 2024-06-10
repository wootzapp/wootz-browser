package org.chromium.chrome.browser.vpn.adapters;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.vpn.activities.VpnServerSelectionActivity.OnServerRegionSelection;
import org.chromium.chrome.browser.vpn.models.WootzVpnServerRegion;
import org.chromium.chrome.browser.vpn.utils.WootzVpnPrefUtils;
import org.chromium.chrome.browser.vpn.utils.WootzVpnUtils;

import java.util.ArrayList;
import java.util.List;

public class WootzVpnServerSelectionAdapter
        extends RecyclerView.Adapter<WootzVpnServerSelectionAdapter.ViewHolder> {
    List<WootzVpnServerRegion> mWootzVpnServerRegions = new ArrayList<>();
    private OnServerRegionSelection mOnServerRegionSelection;

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        LayoutInflater layoutInflater = LayoutInflater.from(parent.getContext());
        View listItem = layoutInflater.inflate(
                R.layout.wootz_vpn_server_selection_item_layout, parent, false);
        return new ViewHolder(listItem);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        final WootzVpnServerRegion vpnServerRegion = mWootzVpnServerRegions.get(position);
        if (vpnServerRegion != null) {
            String locationText =
                    WootzVpnUtils.countryCodeToEmoji(vpnServerRegion.getCountryIsoCode())
                            + " "
                            + vpnServerRegion.getNamePretty();
            holder.serverText.setText(locationText);
            if (WootzVpnPrefUtils.getServerRegion().equals(vpnServerRegion.getName())) {
                holder.serverText.setCompoundDrawablesWithIntrinsicBounds(
                        0, 0, R.drawable.ic_checkbox_checked, 0);
            } else {
                holder.serverText.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, 0);
            }
            holder.serverText.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mOnServerRegionSelection.onServerRegionClick(vpnServerRegion);
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return mWootzVpnServerRegions.size();
    }

    public void setVpnServerRegions(List<WootzVpnServerRegion> vpnServerRegions) {
        this.mWootzVpnServerRegions = vpnServerRegions;
    }

    public void setOnServerRegionSelection(OnServerRegionSelection onServerRegionSelection) {
        this.mOnServerRegionSelection = onServerRegionSelection;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView serverText;

        public ViewHolder(View itemView) {
            super(itemView);
            this.serverText = itemView.findViewById(R.id.server_text);
        }
    }
}