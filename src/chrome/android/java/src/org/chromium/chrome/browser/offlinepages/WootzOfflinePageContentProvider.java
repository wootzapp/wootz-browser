package org.chromium.chrome.browser.offlinepages;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;

import org.chromium.base.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

/**
 * ContentProvider to serve offline MHTML files via content:// URIs.
 * This enables Chromium's built-in MHTML support to work on Android where file:// URLs are blocked.
 */
public class WootzOfflinePageContentProvider extends ContentProvider {
    private static final String TAG = "Kartik: WootzOfflinePageContentProvider";
    private static final String MHTML_MIME_TYPE = "multipart/related";

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        String filePath = uri.getPath();
        if (filePath == null || filePath.isEmpty()) {
            throw new FileNotFoundException("Invalid URI: " + uri);
        }

        if (filePath.startsWith("/")) {
            filePath = filePath.substring(1);
        }

        File file = new File(filePath);
        
        // Security: Validate the file is within app's allowed directories
        try {
            String canonicalFilePath = file.getCanonicalPath();
            String canonicalDataDir = new File(getContext().getApplicationInfo().dataDir).getCanonicalPath();
            
            File externalFilesDir = getContext().getExternalFilesDir(null);
            String canonicalExternalDir = null;
            if (externalFilesDir != null) {
                canonicalExternalDir = externalFilesDir.getCanonicalPath();
            }
            
            boolean isInInternalDir = canonicalFilePath.startsWith(canonicalDataDir);
            boolean isInExternalDir = (canonicalExternalDir != null) && 
                                      canonicalFilePath.startsWith(canonicalExternalDir);
            
            if (!isInInternalDir && !isInExternalDir) {
                Log.e(TAG, "Access denied: " + canonicalFilePath);
                throw new SecurityException("Access denied: path outside app directories");
            }
        } catch (IOException e) {
            throw new FileNotFoundException("Failed to resolve file path: " + e.getMessage());
        }

        if (!file.exists()) {
            throw new FileNotFoundException("File not found: " + filePath);
        }
        
        try {
            return ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
        } catch (IOException e) {
            throw new FileNotFoundException("Cannot open file: " + e.getMessage());
        }
    }

    @Override
    public String getType(Uri uri) {
        return MHTML_MIME_TYPE;
    }

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

