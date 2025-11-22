package org.chromium.chrome.browser.offlinepages;

import android.content.Context;
import android.os.Environment;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

import org.chromium.base.ContextUtils;
import org.chromium.base.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Utility class to provide storage paths for Wootz offline pages.
 */
@JNINamespace("wootz_offline_pages")
public class WootzOfflinePagePathUtils {
    private static final String TAG = "Kartik: WootzOfflinePagePathUtils";
    private static final String OFFLINE_PAGES_DIR = "WootzOfflinePages";
    private static final String OFFLINE_404_FILENAME = "wootz_offline_404_page.mhtml";

    /**
     * Returns the app-specific external storage directory path for offline pages.
     * Path: /storage/emulated/0/Android/data/com.wootzapp.web/files/WootzOfflinePages/
     * This directory is private to the app and doesn't require additional permissions.
     * 
     * @return Absolute path to the offline pages directory, or null if unavailable
     */
    @CalledByNative
    public static String getOfflinePageStoragePath() {
        try {
            Context context = ContextUtils.getApplicationContext();
            File externalFilesDir = context.getExternalFilesDir(null);
            
            if (externalFilesDir == null) {
                Log.e(TAG, "External storage not available");
                return null;
            }
            
            File offlinePagesDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
            
            if (!offlinePagesDir.exists()) {
                if (!offlinePagesDir.mkdirs()) {
                    Log.e(TAG, "Failed to create offline pages directory");
                    return null;
                }
            }
            
            String path = offlinePagesDir.getAbsolutePath();
            Log.i(TAG, "Offline pages storage path: " + path);
            return path;
        } catch (Exception e) {
            Log.e(TAG, "Failed to get offline pages storage path: " + e.getMessage());
           return null;
       }
   }

   /**
    * Exports offline pages from app-specific storage to public Documents directory.
    * Creates a timestamped subdirectory for each export session.
    * Target path: /storage/emulated/0/Documents/WootzOfflinePages/wootz_offline_YYYY-MM-DD_HH-MM-SS/
    * 
    * @return Number of files exported, or -1 on error
    */
   @CalledByNative
   public static int exportOfflinePages() {
       try {
           Context context = ContextUtils.getApplicationContext();
           
           // Source: app-specific external storage
           File externalFilesDir = context.getExternalFilesDir(null);
           if (externalFilesDir == null) {
               Log.e(TAG, "External storage not available for export");
               return -1;
           }
           
           File sourceDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
           if (!sourceDir.exists() || !sourceDir.isDirectory()) {
               Log.e(TAG, "Source directory does not exist: " + sourceDir.getAbsolutePath());
               return 0; // No files to export
           }
           
           // Target: public Documents directory with timestamped subdirectory
           File documentsDir = Environment.getExternalStoragePublicDirectory(
                   Environment.DIRECTORY_DOCUMENTS);
           if (documentsDir == null) {
               Log.e(TAG, "Documents directory not available");
               return -1;
           }
           
           // Create base WootzOfflinePages directory
           File baseTargetDir = new File(documentsDir, OFFLINE_PAGES_DIR);
           if (!baseTargetDir.exists() && !baseTargetDir.mkdirs()) {
               Log.e(TAG, "Failed to create base target directory: " + baseTargetDir.getAbsolutePath());
               return -1;
           }
           
           // Generate timestamp for this export session
           SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd_HH-mm-ss", Locale.US);
           String timestamp = dateFormat.format(new Date());
           String sessionDirName = "wootz_offline_" + timestamp;
           
           // Create timestamped session directory
           File sessionDir = new File(baseTargetDir, sessionDirName);
           if (!sessionDir.mkdirs()) {
               Log.e(TAG, "Failed to create session directory: " + sessionDir.getAbsolutePath());
               return -1;
           }
           
           // Copy all MHTML files to the session directory
           File[] files = sourceDir.listFiles();
           if (files == null || files.length == 0) {
               Log.i(TAG, "No files to export");
               // Delete the empty session directory
               sessionDir.delete();
               return 0;
           }
           
           int exportedCount = 0;
           for (File sourceFile : files) {
               if (!sourceFile.isFile() || !sourceFile.getName().endsWith(".mhtml")) {
                   continue;
               }
               
               File targetFile = new File(sessionDir, sourceFile.getName());
               if (copyFile(sourceFile, targetFile)) {
                   exportedCount++;
                   Log.i(TAG, "Exported: " + sourceFile.getName());
               } else {
                   Log.e(TAG, "Failed to export: " + sourceFile.getName());
               }
           }
           
           Log.i(TAG, "Successfully exported " + exportedCount + " offline pages to " 
                   + sessionDir.getAbsolutePath());
           return exportedCount;
           
       } catch (Exception e) {
           Log.e(TAG, "Failed to export offline pages: " + e.getMessage());
           return -1;
       }
   }

   /**
    * Clears all offline pages from app-specific storage.
    * Note: The special 404 page is preserved and not deleted.
    * 
    * @return Number of files deleted, or -1 on error
    */
   @CalledByNative
   public static int clearOfflinePages() {
       try {
           Context context = ContextUtils.getApplicationContext();
           File externalFilesDir = context.getExternalFilesDir(null);
           
           if (externalFilesDir == null) {
               Log.e(TAG, "External storage not available");
               return -1;
           }
           
           File offlinePagesDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
           
           if (!offlinePagesDir.exists() || !offlinePagesDir.isDirectory()) {
               Log.i(TAG, "Offline pages directory does not exist, nothing to clear");
               return 0;
           }
           
           File[] files = offlinePagesDir.listFiles();
           if (files == null || files.length == 0) {
               Log.i(TAG, "No offline pages to clear");
               return 0;
           }
           
           int deletedCount = 0;
           for (File file : files) {
               // Skip the special 404 page - it should never be deleted
               if (file.getName().equals(OFFLINE_404_FILENAME)) {
                   Log.i(TAG, "Preserving special 404 page: " + file.getName());
                   continue;
               }
               
               if (file.isFile() && file.delete()) {
                   deletedCount++;
                   Log.i(TAG, "Deleted: " + file.getName());
               } else {
                   Log.e(TAG, "Failed to delete: " + file.getName());
               }
           }
           
           Log.i(TAG, "Cleared " + deletedCount + " offline pages from " 
                   + offlinePagesDir.getAbsolutePath() + " (404 page preserved)");
           return deletedCount;
           
       } catch (Exception e) {
           Log.e(TAG, "Failed to clear offline pages: " + e.getMessage());
           return -1;
       }
   }

   /**
    * Gets the full path to the special 404 MHTML page.
    * 
    * @return Path to 404 page, or null on error
    */
   @CalledByNative
   public static String get404PagePath() {
       try {
           Context context = ContextUtils.getApplicationContext();
           File externalFilesDir = context.getExternalFilesDir(null);
           
           if (externalFilesDir == null) {
               Log.e(TAG, "External storage not available");
               return null;
           }
           
           File offlinePagesDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
           File page404 = new File(offlinePagesDir, OFFLINE_404_FILENAME);
           
           return page404.getAbsolutePath();
       } catch (Exception e) {
           Log.e(TAG, "Failed to get 404 page path: " + e.getMessage());
           return null;
       }
   }

   /**
    * Checks if the special 404 page exists.
    * 
    * @return true if 404 page exists, false otherwise
    */
   @CalledByNative
   public static boolean does404PageExist() {
       try {
           String path = get404PagePath();
           if (path == null) {
               return false;
           }
           
           File page404 = new File(path);
           boolean exists = page404.exists() && page404.isFile();
           
           Log.i(TAG, "404 page exists: " + exists + " at " + path);
           return exists;
       } catch (Exception e) {
           Log.e(TAG, "Failed to check 404 page existence: " + e.getMessage());
           return false;
       }
   }

   /**
    * Creates the special 404 MHTML page with the provided content.
    * This page is shown when auto-open is enabled but no offline page exists.
    * 
    * @param mhtmlContent The MHTML content for the 404 page
    * @return true if successful, false otherwise
    */
   @CalledByNative
   public static boolean create404Page(String mhtmlContent) {
       try {
           Context context = ContextUtils.getApplicationContext();
           File externalFilesDir = context.getExternalFilesDir(null);
           
           if (externalFilesDir == null) {
               Log.e(TAG, "External storage not available");
               return false;
           }
           
           File offlinePagesDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
           
           // Create directory if it doesn't exist
           if (!offlinePagesDir.exists() && !offlinePagesDir.mkdirs()) {
               Log.e(TAG, "Failed to create offline pages directory");
               return false;
           }
           
           File page404 = new File(offlinePagesDir, OFFLINE_404_FILENAME);
           
           // Don't overwrite if it already exists
           if (page404.exists()) {
               Log.i(TAG, "404 page already exists at " + page404.getAbsolutePath());
               return true;
           }
           
           // Write MHTML content to file
           FileOutputStream outputStream = null;
           try {
               outputStream = new FileOutputStream(page404);
               outputStream.write(mhtmlContent.getBytes("UTF-8"));
               outputStream.flush();
               
               Log.i(TAG, "Created 404 page at " + page404.getAbsolutePath() 
                       + " (" + mhtmlContent.length() + " bytes)");
               return true;
           } finally {
               if (outputStream != null) {
                   try {
                       outputStream.close();
                   } catch (IOException e) {
                       Log.e(TAG, "Failed to close output stream: " + e.getMessage());
                   }
               }
           }
       } catch (Exception e) {
           Log.e(TAG, "Failed to create 404 page: " + e.getMessage());
           return false;
       }
   }

   /**
    * Searches for an offline page file in subdirectories of WootzOfflinePages.
    * This handles cases where users manually copy exported directories back to app storage.
    * 
    * @param filename The MHTML filename to search for
    * @return Absolute path to the file if found, null otherwise
    */
   @CalledByNative
   public static String findOfflinePageInSubdirectories(String filename) {
       try {
           Context context = ContextUtils.getApplicationContext();
           File externalFilesDir = context.getExternalFilesDir(null);
           
           if (externalFilesDir == null) {
               Log.e(TAG, "External storage not available");
               return null;
           }
           
           File offlinePagesDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
           
           if (!offlinePagesDir.exists() || !offlinePagesDir.isDirectory()) {
               Log.i(TAG, "Offline pages directory does not exist");
               return null;
           }
           
           // Search through all subdirectories
           File[] entries = offlinePagesDir.listFiles();
           if (entries == null) {
               return null;
           }
           
           for (File entry : entries) {
               if (!entry.isDirectory()) {
                   continue;
               }
               
               // Check if the file exists in this subdirectory
               File targetFile = new File(entry, filename);
               if (targetFile.exists() && targetFile.isFile()) {
                   String foundPath = targetFile.getAbsolutePath();
                   Log.i(TAG, "Found offline page in subdirectory: " + foundPath);
                   return foundPath;
               }
               
               // Recursively search nested subdirectories (one level deep)
               File[] subEntries = entry.listFiles();
               if (subEntries != null) {
                   for (File subEntry : subEntries) {
                       if (!subEntry.isDirectory()) {
                           continue;
                       }
                       
                       File nestedFile = new File(subEntry, filename);
                       if (nestedFile.exists() && nestedFile.isFile()) {
                           String foundPath = nestedFile.getAbsolutePath();
                           Log.i(TAG, "Found offline page in nested subdirectory: " + foundPath);
                           return foundPath;
                       }
                   }
               }
           }
           
           Log.i(TAG, "File not found in any subdirectory: " + filename);
           return null;
           
       } catch (Exception e) {
           Log.e(TAG, "Failed to search subdirectories: " + e.getMessage());
           return null;
       }
   }

   /**
    * Copies a file from source to destination.
    * 
    * @param source Source file
    * @param dest Destination file
    * @return true if successful, false otherwise
    */
   private static boolean copyFile(File source, File dest) {
       FileInputStream inputStream = null;
       FileOutputStream outputStream = null;
       
       try {
           inputStream = new FileInputStream(source);
           outputStream = new FileOutputStream(dest);
           
           byte[] buffer = new byte[8192];
           int length;
           
           while ((length = inputStream.read(buffer)) > 0) {
               outputStream.write(buffer, 0, length);
           }
           
           outputStream.flush();
           return true;
           
       } catch (IOException e) {
           Log.e(TAG, "Failed to copy file: " + e.getMessage());
           return false;
       } finally {
           try {
               if (inputStream != null) inputStream.close();
               if (outputStream != null) outputStream.close();
           } catch (IOException e) {
               Log.e(TAG, "Failed to close streams: " + e.getMessage());
           }
       }
   }
}

