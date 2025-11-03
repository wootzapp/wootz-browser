package org.chromium.chrome.browser.offlinepages;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;

import org.chromium.base.Log;

import java.io.File;
import java.io.FileNotFoundException;

/**
 * ContentProvider to serve offline MHTML files via content:// URIs.
 * This enables Chromium's built-in MHTML support to work on Android where file:// URLs are blocked.
 */
public class WootzOfflinePageContentProvider extends ContentProvider {
    private static final String TAG = "Kartik: WootzOfflinePageContentProvider";
    
    // CRITICAL: This MIME type tells Chromium to treat this as an MHTML archive
    private static final String MHTML_MIME_TYPE = "multipart/related";

    @Override
    public boolean onCreate() {
        Log.i(TAG, "WootzOfflinePageContentProvider created");
        return true;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        // Extract file path from the URI
        // URI format: content://org.chromium.chrome.wootz_offline_pages/<absolute_file_path>
        String filePath = uri.getPath();
        if (filePath == null) {
            throw new FileNotFoundException("Invalid URI: " + uri);
        }

        Log.i(TAG, "Opening MHTML file: " + filePath);
        
        File file = new File(filePath);
        if (!file.exists()) {
            throw new FileNotFoundException("MHTML file not found: " + filePath);
        }

        if (!file.canRead()) {
            throw new FileNotFoundException("Cannot read MHTML file: " + filePath);
        }

        // Open file for reading
        return ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
    }

    @Override
    public String getType(Uri uri) {
        // CRITICAL: Return MHTML MIME type so Chromium's NavigationRequest detects it as MHTML
        // This triggers MHTMLArchive creation and subresource loading from the archive
        return MHTML_MIME_TYPE;
    }

    // Not needed for our read-only use case
    @Override
    public Cursor query(Uri uri, String[] projection, String selection,
                       String[] selectionArgs, String sortOrder) {
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        return null;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection,
                     String[] selectionArgs) {
        return 0;
    }
}

