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
    * Target path: /storage/emulated/0/Documents/WootzOfflinePages/
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
           
           // Target: public Documents directory
           File documentsDir = Environment.getExternalStoragePublicDirectory(
                   Environment.DIRECTORY_DOCUMENTS);
           if (documentsDir == null) {
               Log.e(TAG, "Documents directory not available");
               return -1;
           }
           
           File targetDir = new File(documentsDir, OFFLINE_PAGES_DIR);
           if (!targetDir.exists() && !targetDir.mkdirs()) {
               Log.e(TAG, "Failed to create target directory: " + targetDir.getAbsolutePath());
               return -1;
           }
           
           // Copy all MHTML files
           File[] files = sourceDir.listFiles();
           if (files == null || files.length == 0) {
               Log.i(TAG, "No files to export");
               return 0;
           }
           
           int exportedCount = 0;
           for (File sourceFile : files) {
               if (!sourceFile.isFile() || !sourceFile.getName().endsWith(".mhtml")) {
                   continue;
               }
               
               File targetFile = new File(targetDir, sourceFile.getName());
               if (copyFile(sourceFile, targetFile)) {
                   exportedCount++;
                   Log.i(TAG, "Exported: " + sourceFile.getName());
               } else {
                   Log.e(TAG, "Failed to export: " + sourceFile.getName());
               }
           }
           
           Log.i(TAG, "Successfully exported " + exportedCount + " offline pages to " 
                   + targetDir.getAbsolutePath());
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

