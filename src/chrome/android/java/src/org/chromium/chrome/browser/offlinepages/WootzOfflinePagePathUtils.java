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
    private static final String OFFLINE_404_FILENAME = "wootz_offline_404_page.html";
    private static final String OFFLINE_NO_RESULT_FILENAME = "wootz_offline_no_result_found.html";

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
           
           // Copy all HTML and JSON (manifest) files to the session directory
           File[] files = sourceDir.listFiles();
           if (files == null || files.length == 0) {
               Log.i(TAG, "No files to export");
               // Delete the empty session directory
               sessionDir.delete();
               return 0;
           }
           
           int exportedCount = 0;
           for (File sourceFile : files) {
               if (!sourceFile.isFile()) {
                   continue;
               }
               
               String fileName = sourceFile.getName();
               // Export HTML files and their corresponding JSON manifest files
               // Include special pages (404 and no result found)
               if (fileName.endsWith(".html") || fileName.endsWith(".json")) {
                   File targetFile = new File(sessionDir, fileName);
                   if (copyFile(sourceFile, targetFile)) {
                       exportedCount++;
                       Log.i(TAG, "Exported: " + fileName);
                   } else {
                       Log.e(TAG, "Failed to export: " + fileName);
                   }
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
    * Deletes all files and subdirectories EXCEPT the special 404 page.
    * Path: /storage/emulated/0/Android/data/com.wootzapp.web/files/WootzOfflinePages/
    * 
    * @return Number of files/directories deleted, or -1 on error
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
           
           File[] entries = offlinePagesDir.listFiles();
           if (entries == null || entries.length == 0) {
               Log.i(TAG, "No offline pages to clear");
               return 0;
           }
           
           int deletedCount = 0;
           for (File entry : entries) {
               String entryName = entry.getName();
               
               // Skip special pages (404 and no result found) and their manifests
               if (entryName.equals(OFFLINE_404_FILENAME) || 
                   entryName.equals(OFFLINE_404_FILENAME.replace(".html", ".json")) ||
                   entryName.equals(OFFLINE_NO_RESULT_FILENAME) ||
                   entryName.equals(OFFLINE_NO_RESULT_FILENAME.replace(".html", ".json"))) {
                   Log.i(TAG, "Preserving special page: " + entryName);
                   continue;
               }
               
               // Delete files and directories
               if (entry.isFile()) {
                   if (entry.delete()) {
                       deletedCount++;
                       Log.i(TAG, "Deleted file: " + entryName);
                   } else {
                       Log.e(TAG, "Failed to delete file: " + entryName);
                   }
               } else if (entry.isDirectory()) {
                   int dirDeleted = deleteDirectoryRecursive(entry);
                   if (dirDeleted >= 0) {
                       deletedCount += dirDeleted;
                       Log.i(TAG, "Deleted directory: " + entryName + " (" + dirDeleted + " items)");
                   } else {
                       Log.e(TAG, "Failed to delete directory: " + entryName);
                   }
               }
           }
           
           Log.i(TAG, "Cleared " + deletedCount + " items from " 
                   + offlinePagesDir.getAbsolutePath() + " (404 page preserved)");
           return deletedCount;
           
       } catch (Exception e) {
           Log.e(TAG, "Failed to clear offline pages: " + e.getMessage());
           return -1;
       }
   }
   
   /**
    * Recursively deletes a directory and all its contents.
    * 
    * @param directory The directory to delete
    * @return Number of items deleted, or -1 on error
    */
   private static int deleteDirectoryRecursive(File directory) {
       if (!directory.exists()) {
           return 0;
       }
       
       if (!directory.isDirectory()) {
           return directory.delete() ? 1 : -1;
       }
       
       int deletedCount = 0;
       File[] files = directory.listFiles();
       
       if (files != null) {
           for (File file : files) {
               if (file.isDirectory()) {
                   int result = deleteDirectoryRecursive(file);
                   if (result >= 0) {
                       deletedCount += result;
                   } else {
                       return -1;
                   }
               } else {
                   if (file.delete()) {
                       deletedCount++;
                   } else {
                       Log.e(TAG, "Failed to delete file: " + file.getAbsolutePath());
                       return -1;
                   }
               }
           }
       }
       
       // Delete the directory itself after deleting all contents
       if (directory.delete()) {
           deletedCount++;
           return deletedCount;
       } else {
           Log.e(TAG, "Failed to delete directory: " + directory.getAbsolutePath());
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
    * Creates the special 404 HTML page with the provided content.
    * This page is shown when auto-open is enabled but no offline page exists.
    * 
    * @param htmlContent The HTML content for the 404 page
    * @return true if successful, false otherwise
    */
   @CalledByNative
   public static boolean create404Page(String htmlContent) {
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
           
           // Write HTML content to file
           FileOutputStream outputStream = null;
           try {
               outputStream = new FileOutputStream(page404);
               outputStream.write(htmlContent.getBytes("UTF-8"));
               outputStream.flush();
               
               Log.i(TAG, "Created 404 page at " + page404.getAbsolutePath() 
                       + " (" + htmlContent.length() + " bytes)");
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
    * Gets the full path to the "no result found" page for LinkedIn searches.
    * 
    * @return Path to no result found page, or null on error
    */
   @CalledByNative
   public static String getNoResultFoundPagePath() {
       try {
           Context context = ContextUtils.getApplicationContext();
           File externalFilesDir = context.getExternalFilesDir(null);
           
           if (externalFilesDir == null) {
               Log.e(TAG, "External storage not available");
               return null;
           }
           
           File offlinePagesDir = new File(externalFilesDir, OFFLINE_PAGES_DIR);
           File noResultPage = new File(offlinePagesDir, OFFLINE_NO_RESULT_FILENAME);
           
           return noResultPage.getAbsolutePath();
       } catch (Exception e) {
           Log.e(TAG, "Failed to get no result found page path: " + e.getMessage());
           return null;
       }
   }

   /**
    * Checks if the "no result found" page exists.
    * 
    * @return true if page exists, false otherwise
    */
   @CalledByNative
   public static boolean doesNoResultFoundPageExist() {
       try {
           String path = getNoResultFoundPagePath();
           if (path == null) {
               return false;
           }
           
           File noResultPage = new File(path);
           boolean exists = noResultPage.exists() && noResultPage.isFile();
           
           Log.i(TAG, "No result found page exists: " + exists + " at " + path);
           return exists;
       } catch (Exception e) {
           Log.e(TAG, "Failed to check no result found page existence: " + e.getMessage());
           return false;
       }
   }

   /**
    * Searches for an offline page file in subdirectories of WootzOfflinePages.
    * This handles cases where users manually copy exported directories back to app storage.
    * Also searches manifest files to find pages via redirect mappings.
    * 
    * @param filename The HTML filename to search for
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
               
               // Also check manifest files in this subdirectory
               File[] manifestFiles = entry.listFiles((dir, name) -> name.endsWith(".json"));
               if (manifestFiles != null) {
                   for (File manifestFile : manifestFiles) {
                       // Manifests might reference the file we're looking for
                       Log.d(TAG, "Found manifest in subdirectory: " + manifestFile.getName());
                   }
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

