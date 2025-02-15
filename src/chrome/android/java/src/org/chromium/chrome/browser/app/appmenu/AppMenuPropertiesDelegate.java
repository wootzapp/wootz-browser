package org.chromium.chrome.browser.app.appmenu;

import android.view.Menu;
import android.view.MenuItem;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ChromeActivity;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.chrome.browser.tabmodel.TabModelSelectorTabModelObserver;
import org.chromium.chrome.browser.tabmodel.TabModelSelectorTabObserver;
import org.chromium.content_public.browser.NavigationParams;
import org.chromium.url.GURL;

public class AppMenuPropertiesDelegate implements AppMenuDelegate {
    private static final int EXTENSIONS_MENU_ID = R.id.extensions_menu_id;

    @Override
    public void prepareMenu(Menu menu, ChromeActivity activity) {
        super.prepareMenu(menu);
        
        menu.add(Menu.NONE, EXTENSIONS_MENU_ID, Menu.NONE, R.string.menu_extensions)
            .setIcon(R.drawable.extensions_icon);
    }

    @Override
    public boolean onItemClick(MenuItem item, ChromeActivity activity) {
        if (item.getItemId() == EXTENSIONS_MENU_ID) {
            openExtensionsPage(activity);
            return true;
        }
        return super.onItemClick(item, activity);
    }

    private void openExtensionsPage(ChromeActivity activity) {
        String url = "chrome://extensions";
        activity.getTabCreator(false).createNewTab(
                new NavigationParams.Builder()
                        .setUrl(new GURL(url))
                        .setInitiallyHidden(false)
                        .build());
    }
}