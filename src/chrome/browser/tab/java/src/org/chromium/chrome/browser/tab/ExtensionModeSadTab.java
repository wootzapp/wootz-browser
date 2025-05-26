package org.chromium.chrome.browser.tab;

import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class ExtensionModeSadTab extends SadTab {

    public ExtensionModeSadTab(Tab tab) {
        super(tab);
    }

    @Override
    protected View createView(
            Context context,
            final Runnable suggestionAction,
            Runnable buttonAction,
            boolean showSendFeedbackView,
            boolean isIncognito) {
        
        // Call the parent implementation to get the base view
        View sadTabView = super.createView(
                context, suggestionAction, buttonAction, showSendFeedbackView, isIncognito);
        
        TextView titleText = (TextView) sadTabView.findViewById(R.id.sad_tab_title);
        titleText.setText("Extension Developer Mode is Disabled");
        
        TextView messageText = (TextView) sadTabView.findViewById(R.id.sad_tab_message);
        messageText.setText("Please enable developer mode in settings > Extension Developer Mode");
        
        Button button = (Button) sadTabView.findViewById(R.id.sad_tab_button);
        button.setText("Enable Developer Mode");

        return sadTabView;
    }
    
}