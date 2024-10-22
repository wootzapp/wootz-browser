package org.chromium.chrome.browser.extensions;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;
import android.util.Log;

public class BrowsingDataProvider extends ContentProvider {
    private static final String TAG = "BrowsingDataProvider";
    private static final String DATABASE_NAME = "dummy.db";
    private static final int DATABASE_VERSION = 1;

    private DatabaseHelper dbHelper;

    @Override
    public boolean onCreate() {
        Log.d(TAG, "onCreate called");
        dbHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
        Log.d(TAG, "query called with URI: " + uri);
        return null;
    }

    @Override
    public String getType(Uri uri) {
        Log.d(TAG, "getType called with URI: " + uri);
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        Log.d(TAG, "insert called with URI: " + uri);
        return null;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        Log.d(TAG, "delete called with URI: " + uri);
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        Log.d(TAG, "update called with URI: " + uri);
        return 0;
    }

    private static class DatabaseHelper extends SQLiteOpenHelper {
        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
            Log.d(TAG, "DatabaseHelper constructor called");
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            Log.d(TAG, "DatabaseHelper onCreate called");
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            Log.d(TAG, "DatabaseHelper onUpgrade called");
        }
    }
}