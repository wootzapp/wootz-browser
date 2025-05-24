package org.chromium.components.action_url;

import android.util.Log;
import org.json.JSONArray;
import org.json.JSONObject;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Helper class for URL pattern matching and path variable handling.
 */
public class UrlPatternMatcher {
    
    /**
     * Finds a matching action URL from the action.json content based on the URL path.
     * 
     * @param jsonContent The content of the action.json file
     * @param originalUrl The original URL being processed
     * @param path The path part of the URL
     * @param query The query string of the URL (with '?' if present)
     * @return The matching action URL, or empty string if no match is found
     */
    public static String findMatchingActionUrl(String jsonContent, String originalUrl, String path, String query) {
        try {
            // Parse the JSON content
            JSONObject json = new JSONObject(jsonContent);
            
            // Check if the JSON has the expected format with rules array
            if (!json.has("rules")) {
                Log.e("::Unfurling:::: ", "JSON does not contain rules array");
                return "";
            }
            
            // Get the rules array
            JSONArray rules = json.getJSONArray("rules");
            
            // Extract all rules into a list for sorting
            List<JSONObject> rulesList = new ArrayList<>();
            for (int i = 0; i < rules.length(); i++) {
                rulesList.add(rules.getJSONObject(i));
            }
            
            Log.e("::Unfurling:::: ", "Found " + rulesList.size() + " rules to match against");
            
            // Sort rules by specificity (more segments in pathPattern first)
            Collections.sort(rulesList, (a, b) -> {
                try {
                    String patternA = a.getString("pathPattern");
                    String patternB = b.getString("pathPattern");
                    return countSegments(patternB) - countSegments(patternA);
                } catch (Exception e) {
                    return 0;
                }
            });
            
            // Try to match each rule's pattern
            for (JSONObject rule : rulesList) {
                try {
                    String pattern = rule.getString("pathPattern");
                    Log.e("::Unfurling:::: ", "Trying to match pattern: " + pattern);
                    
                    if (pathMatchesPattern(path, pattern)) {
                        // Get the API endpoint for this pattern
                        String apiEndpoint = rule.getString("apiPath");
                        Log.e("::Unfurling:::: ", "Pattern matched! API endpoint: " + apiEndpoint);
                        
                        // Apply path variables if needed
                        String finalApiEndpoint = applyPathVariables(apiEndpoint, pattern, path);
                        Log.e("::Unfurling:::: ", "Final API endpoint with variables: " + finalApiEndpoint);
                        
                        // Extract the origin from the original URL
                        URL url = new URL(originalUrl);
                        String origin = url.getProtocol() + "://" + url.getHost();
                        
                        // Construct the full action URL
                        String actionUrl = origin + finalApiEndpoint + query;
                        Log.e("::Unfurling:::: ", "Constructed action URL: " + actionUrl);
                        
                        return actionUrl;
                    }
                } catch (Exception e) {
                    Log.e("::Unfurling:::: ", "Error processing rule: " + e.getMessage());
                    // Continue to next rule
                }
            }
            
            Log.e("::Unfurling:::: ", "No matching pattern found for path: " + path);
            return "";
            
        } catch (Exception e) {
            Log.e("::Unfurling:::: ", "Error finding matching action URL: " + e.getMessage(), e);
            return "";
        }
    }

    /**
     * Counts the number of segments in a pattern.
     */
    private static int countSegments(String pattern) {
        if (pattern == null || pattern.isEmpty()) {
            return 0;
        }
        
        // Normalize the pattern (remove leading slash)
        String normalized = pattern.startsWith("/") ? pattern.substring(1) : pattern;
        return normalized.isEmpty() ? 0 : normalized.split("/").length;
    }

    /**
     * Check if a path matches a pattern with wildcards.
     */
    private static boolean pathMatchesPattern(String path, String pattern) {
        // Normalize paths (remove leading/trailing slashes)
        String normalizedPath = path;
        if (normalizedPath.startsWith("/")) {
            normalizedPath = normalizedPath.substring(1);
        }
        if (normalizedPath.endsWith("/")) {
            normalizedPath = normalizedPath.substring(0, normalizedPath.length() - 1);
        }
        
        String normalizedPattern = pattern;
        if (normalizedPattern.startsWith("/")) {
            normalizedPattern = normalizedPattern.substring(1);
        }
        if (normalizedPattern.endsWith("/")) {
            normalizedPattern = normalizedPattern.substring(0, normalizedPattern.length() - 1);
        }
        
        // Handle root path
        if (normalizedPath.isEmpty() && normalizedPattern.isEmpty()) {
            return true;
        }
        
        // Split into segments
        String[] pathSegments = normalizedPath.isEmpty() ? new String[0] : normalizedPath.split("/");
        String[] patternSegments = normalizedPattern.isEmpty() ? new String[0] : normalizedPattern.split("/");
        
        // Simple case: catch-all pattern
        if (patternSegments.length == 1 && "**".equals(patternSegments[0])) {
            return true;
        }
        
        // Use recursive matching helper
        return matchSegmentsRecursive(pathSegments, patternSegments, 0, 0);
    }

    /**
     * Recursive helper for matching path segments to pattern segments.
     */
    private static boolean matchSegmentsRecursive(String[] pathSegments, String[] patternSegments, 
                                          int pathIndex, int patternIndex) {
        // Both arrays are exhausted - match found
        if (pathIndex >= pathSegments.length && patternIndex >= patternSegments.length) {
            return true;
        }
        
        // Pattern exhausted but path remains - no match
        if (patternIndex >= patternSegments.length) {
            return false;
        }
        
        // Get current pattern segment
        String patternSegment = patternSegments[patternIndex];
        
        // Handle "**" wildcard (matches zero or more segments)
        if (patternSegment.equals("**")) {
            // Last segment is ** - matches everything remaining
            if (patternIndex == patternSegments.length - 1) {
                return true;
            }
            
            // Try matching the rest of the pattern at different positions in the path
            for (int i = pathIndex; i <= pathSegments.length; i++) {
                if (matchSegmentsRecursive(pathSegments, patternSegments, i, patternIndex + 1)) {
                    return true;
                }
            }
            return false;
        }
        
        // Path exhausted but pattern remains - only match if remaining pattern segments are all "**"
        if (pathIndex >= pathSegments.length) {
            // Check if all remaining pattern segments are "**"
            for (int i = patternIndex; i < patternSegments.length; i++) {
                if (!patternSegments[i].equals("**")) {
                    return false;
                }
            }
            return true;
        }
        
        // Get current path segment
        String pathSegment = pathSegments[pathIndex];
        
        // Handle "*" wildcard (matches exactly one segment)
        if (patternSegment.equals("*")) {
            return matchSegmentsRecursive(pathSegments, patternSegments, pathIndex + 1, patternIndex + 1);
        }
        
        // Exact match required
        if (patternSegment.equals(pathSegment)) {
            return matchSegmentsRecursive(pathSegments, patternSegments, pathIndex + 1, patternIndex + 1);
        }
        
        // No match
        return false;
    }

    /**
     * Apply pattern variables to API path.
     */
    private static String applyPathVariables(String apiPath, String pattern, String path) {
        // If no wildcards in API path, return as is
        if (!apiPath.contains("*")) {
            return apiPath;
        }
        
        Log.e("::Unfurling:::: ", "Applying variables to API path: " + apiPath);
        Log.e("::Unfurling:::: ", "Pattern: " + pattern + ", Path: " + path);
        
        // Normalize paths
        String normalizedPath = path.startsWith("/") ? path.substring(1) : path;
        String normalizedPattern = pattern.startsWith("/") ? pattern.substring(1) : pattern;
        
        // Get pattern and path segments
        String[] pathSegments = normalizedPath.split("/");
        String[] patternSegments = normalizedPattern.split("/");
        
        // Extract values that match wildcards
        List<String> wildcardValues = extractWildcardValues(pathSegments, patternSegments);
        Log.e("::Unfurling:::: ", "Extracted " + wildcardValues.size() + " wildcard values");
        
        // Replace wildcards in API path
        String result = apiPath;
        for (String value : wildcardValues) {
            if (result.contains("*")) {
                Log.e("::Unfurling:::: ", "Replacing wildcard with: " + value);
                result = result.replaceFirst("\\*", value);
            }
        }
        
        // Check for any remaining wildcards and remove them
        if (result.contains("*")) {
            Log.e("::Unfurling:::: ", "Removing trailing wildcards from: " + result);
            result = result.replaceAll("\\*", "");
        }
        
        Log.e("::Unfurling:::: ", "Final API path after variable application: " + result);
        
        return result;
    }

    /**
     * Extract values that match wildcards in pattern.
     */
    private static List<String> extractWildcardValues(String[] pathSegments, String[] patternSegments) {
        List<String> values = new ArrayList<>();
        
        int pathIndex = 0;
        for (int i = 0; i < patternSegments.length && pathIndex < pathSegments.length; i++) {
            String patternSegment = patternSegments[i];
            
            if (patternSegment.equals("*")) {
                // Capture single segment
                values.add(pathSegments[pathIndex]);
                pathIndex++;
            } else if (patternSegment.equals("**")) {
                // Find how many segments to capture
                int remaining = 0;
                if (i < patternSegments.length - 1) {
                    // Find next non-wildcard segment in pattern
                    String nextSegment = null;
                    for (int j = i + 1; j < patternSegments.length; j++) {
                        if (!patternSegments[j].equals("*") && !patternSegments[j].equals("**")) {
                            nextSegment = patternSegments[j];
                            break;
                        }
                    }
                    
                    if (nextSegment != null) {
                        // Find where this segment appears in the path
                        int nextIndex = -1;
                        for (int j = pathIndex; j < pathSegments.length; j++) {
                            if (pathSegments[j].equals(nextSegment)) {
                                nextIndex = j;
                                break;
                            }
                        }
                        
                        if (nextIndex != -1) {
                            // Capture segments up to the next match
                            StringBuilder segments = new StringBuilder();
                            for (int j = pathIndex; j < nextIndex; j++) {
                                if (j > pathIndex) {
                                    segments.append("/");
                                }
                                segments.append(pathSegments[j]);
                            }
                            values.add(segments.toString());
                            pathIndex = nextIndex;
                            continue;
                        }
                    }
                }
                
                // If we're here, capture all remaining segments
                StringBuilder segments = new StringBuilder();
                for (int j = pathIndex; j < pathSegments.length; j++) {
                    if (j > pathIndex) {
                        segments.append("/");
                    }
                    segments.append(pathSegments[j]);
                }
                values.add(segments.toString());
                break;
            } else if (patternSegment.equals(pathSegments[pathIndex])) {
                // Skip exact matches
                pathIndex++;
            }
        }
        
        return values;
    }
}
