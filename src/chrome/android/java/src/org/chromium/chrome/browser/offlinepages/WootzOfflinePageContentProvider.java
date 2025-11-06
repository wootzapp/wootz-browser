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
        if (filePath == null || filePath.isEmpty()) {
            throw new FileNotFoundException("Invalid URI: " + uri);
        }

        // Remove leading slash if present
        if (filePath.startsWith("/")) {
            filePath = filePath.substring(1);
        }

        File file = new File(filePath);
        
        // Security: Prevent path traversal attacks by validating the canonical path
        // is within the app's data directory or external files directory
        try {
            String canonicalFilePath = file.getCanonicalPath();
            
            // Check internal data directory
            String dataDir = getContext().getApplicationInfo().dataDir;
            File dataDirFile = new File(dataDir);
            String canonicalDataDir = dataDirFile.getCanonicalPath();
            
            // Check external files directory
            File externalFilesDir = getContext().getExternalFilesDir(null);
            String canonicalExternalDir = null;
            if (externalFilesDir != null) {
                canonicalExternalDir = externalFilesDir.getCanonicalPath();
            }
            
            Log.i(TAG, "Validating path - File: " + canonicalFilePath 
                  + ", InternalDir: " + canonicalDataDir 
                  + ", ExternalDir: " + canonicalExternalDir);
            
            // Allow access if file is in either internal data dir or external files dir
            boolean isInInternalDir = canonicalFilePath.startsWith(canonicalDataDir);
            boolean isInExternalDir = (canonicalExternalDir != null) && 
                                      canonicalFilePath.startsWith(canonicalExternalDir);
            
            if (!isInInternalDir && !isInExternalDir) {
                Log.e(TAG, "Path traversal attempt detected: " + canonicalFilePath 
                      + " is outside app directories");
                throw new SecurityException("Access denied: path outside app directory");
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to resolve canonical path: " + e.getMessage());
            throw new FileNotFoundException("Failed to resolve file path: " + e.getMessage());
        }

        Log.i(TAG, "Opening MHTML file: " + filePath);
        
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

