package org.chromium.chrome.browser.vpn.timer;

import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.chrome.R;

public class TimerItemViewHolder extends RecyclerView.ViewHolder {
    final TextView mTimerActionText;
    final ImageView mTimerActionImage;

    public TimerItemViewHolder(@NonNull View itemView) {
        super(itemView);
        this.mTimerActionText = itemView.findViewById(R.id.timer_action_text);
        this.mTimerActionImage = itemView.findViewById(R.id.timer_action_image);
    }
}