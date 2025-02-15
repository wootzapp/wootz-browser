package org.chromium.chrome.browser.extensions;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import org.chromium.chrome.R;
import org.chromium.components.browser_ui.widget.ChromeToolbar;

public class ExtensionStoreActivity extends AppCompatActivity {
    private ChromeToolbar mToolbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_extension_store);
        
        mToolbar = findViewById(R.id.toolbar);
        setSupportActionBar(mToolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setTitle(R.string.extension_store_title);
    }

    @Override
    public boolean onSupportNavigateUp() {
        finish();
        return true;
    }
} 