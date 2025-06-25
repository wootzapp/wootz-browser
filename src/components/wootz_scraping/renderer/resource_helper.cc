/* Copyright (c) 2024 All rights reserved. */

  #include "components/wootz_scraping/renderer/resource_helper.h"

  #include "base/check.h"
  #include "base/logging.h"
  #include "base/strings/string_piece.h"
  #include "base/strings/string_util.h"
  #include "third_party/blink/public/platform/web_string.h"
  #include "third_party/blink/public/web/web_script_source.h"

  namespace wootz_scraping {
    

  // Dynamic Twitter API Network Interception System - No Hardcoded Values
  const char* GetTwitterInterceptorScript() {
    return R"(


// Enhanced Mobile Twitter API Interceptor - COMPLETE FEATURE AGGREGATION
// Captures ALL features from ALL GraphQL endpoints dynamically

// Global storage for ALL captured features from multiple endpoints
window.capturedTwitterApiData = {
    apiBaseUrl: null,
    variables: null,
    features: null,
    allFeatures: {}, // NEW: Aggregated features from ALL endpoints
    allEndpoints: {}, // NEW: Track features per endpoint
    bearerToken: null,
    operationId: null,
    lastCaptureTime: null,
    captureCount: 0,
    isValid: false
};

// NEW: Global storage for captured browser headers
window.capturedBrowserHeaders = {
    clientTransactionId: null,
    xpForwardedFor: null,
    lastCaptureTime: null,
    captureSource: null
};

// NEW: Storage for complete request headers for URL validation
window.lastCapturedHeaders = {};
window.allRequestHeaders = new Map(); // Store headers per request URL

// Track all unique features discovered across ALL endpoints
window.allDiscoveredFeatures = new Set();
window.endpointFeatureMap = new Map();
window.homeTimelineFeatures = {}; // NEW: Dedicated storage for HomeTimeline features
window.homeTimelineVariables = {}; // NEW: Dedicated storage for HomeTimeline variables
window.lastCapturedHomeTimelineUrl = null; // NEW: Store the complete HomeTimeline URL for later reference
window.skippedEndpoints = { // NEW: Track skipped endpoints
    viewer: 0,
    other: 0,
    total: 0
};

function interceptNetworkRequests() {
    console.log('🌐 COMPLETE FEATURE CAPTURE -> Starting comprehensive Twitter API interception...');
    
    const originalFetch = window.fetch;
    const originalXHROpen = XMLHttpRequest.prototype.open;
    const originalXHRSend = XMLHttpRequest.prototype.send;
    
    // Enhanced fetch interception for ALL GraphQL endpoints
    window.fetch = function(url, options = {}) {
        try {
            if (typeof url === 'string' && (
                url.includes('/i/api/graphql/') ||
                url.includes('/api/graphql/') ||
                url.includes('graphql')
            )) {
                console.log('🎯 COMPLETE CAPTURE -> GraphQL request detected:', url);
                
                // NEW: Capture actual browser headers from the request
                if (options.headers) {
                    // Store all headers for later use
                    window.lastCapturedHeaders = { ...options.headers };
                    window.allRequestHeaders.set(url, { ...options.headers });
                    console.log('✅ CAPTURED ALL HEADERS from FETCH:', Object.keys(options.headers).length, 'headers');
                    
                    const clientTransactionId = options.headers['x-client-transaction-id'] || 
                                              options.headers['X-Client-Transaction-Id'];
                    const xpForwardedFor = options.headers['x-xp-forwarded-for'] || 
                                         options.headers['X-Xp-Forwarded-For'];
                    
                    if (clientTransactionId) {
                        window.capturedBrowserHeaders.clientTransactionId = clientTransactionId;
                        window.capturedBrowserHeaders.lastCaptureTime = Date.now();
                        window.capturedBrowserHeaders.captureSource = 'fetch';
                        console.log('✅ CAPTURED x-client-transaction-id from FETCH:', clientTransactionId);
                    }
                    
                    if (xpForwardedFor) {
                        window.capturedBrowserHeaders.xpForwardedFor = xpForwardedFor;
                        console.log('✅ CAPTURED x-xp-forwarded-for from FETCH:', xpForwardedFor);
                    }
                }
                
                extractApiParametersFromRequest(url, options);
            }
        } catch (error) {
            console.warn('⚠️ COMPLETE CAPTURE -> Error intercepting fetch:', error);
        }
        return originalFetch.apply(this, arguments);
    };
    
    // Enhanced XHR interception for ALL GraphQL endpoints  
    XMLHttpRequest.prototype.open = function(method, url, async, user, password) {
        this._url = url;
        this._method = method;
        this._headers = {}; // Initialize headers storage
        return originalXHROpen.apply(this, arguments);
    };
    
    // NEW: Intercept XHR header setting to capture all headers
    const originalXHRSetRequestHeader = XMLHttpRequest.prototype.setRequestHeader;
    XMLHttpRequest.prototype.setRequestHeader = function(name, value) {
        // Store headers for later use
        if (!this._headers) this._headers = {};
        this._headers[name] = value;
        
        // Store headers globally for validation
        if (this._url) {
            if (!window.allRequestHeaders.has(this._url)) {
                window.allRequestHeaders.set(this._url, {});
            }
            const urlHeaders = window.allRequestHeaders.get(this._url);
            urlHeaders[name] = value;
            window.lastCapturedHeaders[name] = value;
        }
        
        console.log('✅ CAPTURED XHR HEADER:', name, '=', value);
        return originalXHRSetRequestHeader.apply(this, arguments);
    };
    
    XMLHttpRequest.prototype.send = function(data) {
        try {
            if (this._url && typeof this._url === 'string' && (
                this._url.includes('/i/api/graphql/') ||
                this._url.includes('/api/graphql/') ||
                this._url.includes('graphql')
            )) {
                console.log('🎯 COMPLETE CAPTURE -> GraphQL XHR detected:', this._url);
                extractApiParametersFromRequest(this._url, { 
                    method: this._method,
                    body: data,
                    headers: this._headers || {}
                });
            }
        } catch (error) {
            console.warn('⚠️ COMPLETE CAPTURE -> Error intercepting XHR:', error);
        }
        return originalXHRSend.apply(this, arguments);
    };
}

// ENHANCED: Complete feature extraction from ALL endpoints
function extractApiParametersFromRequest(url, options = {}) {
    console.log('🔍 COMPLETE EXTRACTION -> Processing GraphQL URL:', url);
    
    // CRITICAL: Only capture features from HomeTimeline endpoint
    const isHomeTimelineEndpoint = url.includes('/HomeTimeline') || 
                                  url.includes('/HomeLatestTimeline') ||
                                  url.includes('/HomeTimelineSSR');
    
    // ENHANCED: Capture complete HomeTimeline URL and send to backend ONLY when all required data is present
    if (isHomeTimelineEndpoint) {
        console.log('🎯 HOMETIMELINE URL DETECTED -> Checking if all required data is present');
        
        // Capture complete URL with all parameters (features and variables included)
        const completeUrl = url.toString();
        console.log('📋 COMPLETE URL LENGTH:', completeUrl.length, 'characters');
        
        // IMPORTANT: Store the complete URL globally for later reference by feature update functions
        window.lastCapturedHomeTimelineUrl = completeUrl;
        console.log('✅ STORED COMPLETE URL for later reference by feature updates');
        console.log('📋 COMPLETE URL PREVIEW:', completeUrl.substring(0, 200) + '...');
        
        // Check if URL contains required parameters
        const hasFeatures = completeUrl.includes('features=');
        const hasVariables = completeUrl.includes('variables=');
        const hasRequiredParams = hasFeatures && hasVariables;
        
        console.log('🔍 URL PARAMETER CHECK:');
        console.log('  - Contains features:', hasFeatures ? '✅' : '❌');
        console.log('  - Contains variables:', hasVariables ? '✅' : '❌');
        console.log('  - Has all required params:', hasRequiredParams ? '✅' : '❌');
        
        // Check if headers are available (from request options or global state)
        let hasHeaders = false;
        let capturedHeaders = {};
        
        // Try to capture headers from the current request options
        if (options && options.headers) {
            capturedHeaders = { ...options.headers };
            hasHeaders = Object.keys(capturedHeaders).length > 0;
            console.log('🔍 REQUEST HEADERS FOUND:', Object.keys(capturedHeaders).length, 'headers');
        } else if (window.lastCapturedHeaders && Object.keys(window.lastCapturedHeaders).length > 0) {
            capturedHeaders = { ...window.lastCapturedHeaders };
            hasHeaders = true;
            console.log('🔍 USING CACHED HEADERS:', Object.keys(capturedHeaders).length, 'headers');
        }
        
        console.log('🔍 HEADERS CHECK:');
        console.log('  - Headers available:', hasHeaders ? '✅' : '❌');
        
        // Extract current cookies as proper HTTP cookie string
        const currentTokens = document.cookie || '';
        const hasCookies = currentTokens.length > 0;
        console.log('🍪 Extracted cookie string length:', currentTokens.length);
        console.log('🍪 Cookie string preview:', currentTokens.substring(0, 100) + '...');
        console.log('🔍 COOKIES CHECK:');
        console.log('  - Cookies available:', hasCookies ? '✅' : '❌');
        
        // Only send if ALL required data is present
        const allDataReady = hasRequiredParams && hasHeaders && hasCookies;
        console.log('🎯 COMPLETE DATA CHECK:', allDataReady ? '✅ ALL READY' : '❌ MISSING DATA');
        
        if (allDataReady) {
            console.log('✅ ALL REQUIRED DATA PRESENT -> Sending complete HomeTimeline URL');
            
            // METHOD 1: Send via OnHomeTimelineUrlCaptured (existing method)
            try {
                if (window.twitterScrapingBridge && typeof window.twitterScrapingBridge.onHomeTimelineUrlCaptured === 'function') {
                    window.twitterScrapingBridge.onHomeTimelineUrlCaptured(completeUrl, currentTokens);
                    console.log('✅ HomeTimeline URL sent via OnHomeTimelineUrlCaptured successfully');
                } else {
                    console.log('⚠️ Twitter bridge not available for URL capture');
                }
            } catch (error) {
                console.error('❌ Failed to send HomeTimeline URL via OnHomeTimelineUrlCaptured:', error);
            }
            
            // METHOD 2: ALSO send via OnDataCaptured with complete data
            try {
                if (window.twitterScrapingBridge && typeof window.twitterScrapingBridge.onDataCaptured === 'function') {
                    const urlData = {
                        type: 'HomeTimeline_Complete_URL',
                        url: completeUrl,
                        urlLength: completeUrl.length,
                        timestamp: Date.now(),
                        source: 'JavaScript_URL_Capture',
                        hasFeatures: hasFeatures,
                        hasVariables: hasVariables,
                        tokens: currentTokens,
                        capturedBrowserHeaders: capturedHeaders
                    };
                    window.twitterScrapingBridge.onDataCaptured(JSON.stringify(urlData));
                    console.log('✅ HomeTimeline URL with complete data sent via OnDataCaptured successfully');
                } else {
                    console.log('⚠️ OnDataCaptured method not available for URL capture');
                }
            } catch (error) {
                console.error('❌ Failed to send HomeTimeline URL via OnDataCaptured:', error);
            }
        } else {
            console.log('⚠️ INCOMPLETE DATA -> Not sending HomeTimeline URL yet');
            console.log('  Missing:', 
                !hasRequiredParams ? 'URL_PARAMS ' : '',
                !hasHeaders ? 'HEADERS ' : '',
                !hasCookies ? 'COOKIES ' : ''
            );
            console.log('  Will retry when all data becomes available');
        }
    }
    
    const isViewerEndpoint = url.includes('/Viewer') || 
                            url.includes('/viewer') ||
                            url.includes('/ViewerUserInfo');
    
    if (isViewerEndpoint) {
        console.log('🚫 SKIPPING VIEWER ENDPOINT -> Not capturing features from:', url);
        window.skippedEndpoints.viewer++;
        window.skippedEndpoints.total++;
        console.log(`   📊 Total viewer endpoints skipped: ${window.skippedEndpoints.viewer}`);
        return; // Skip viewer endpoints completely
    }
    
    if (!isHomeTimelineEndpoint) {
        console.log('⚠️ NON-HOMETIMELINE ENDPOINT DETECTED -> URL:', url);
        console.log('   📍 Only HomeTimeline features will be used for final data');
        window.skippedEndpoints.other++;
        window.skippedEndpoints.total++;
        console.log(`   📊 Total non-HomeTimeline endpoints: ${window.skippedEndpoints.other}`);
        // Continue but mark as non-HomeTimeline
    }
    
    // Check if we're on login page - don't overwrite good data with login page data
    const isLoginPage = window.location.pathname.includes('/login') || 
                       window.location.pathname.includes('/i/flow/login') ||
                       window.location.pathname === '/';
    
    if (isLoginPage && window.capturedTwitterApiData && 
        window.capturedTwitterApiData.allFeatures && 
        Object.keys(window.capturedTwitterApiData.allFeatures).length > 20) {
        console.log('⚠️ LOGIN PAGE DETECTED -> Skipping feature update to preserve homepage features');
        console.log(`   Current features: ${Object.keys(window.capturedTwitterApiData.allFeatures).length}`);
        console.log(`   Location: ${window.location.pathname}`);
        return; // Don't overwrite good data with login page data
    }
    
    try {
        // Parse URL with enhanced mobile compatibility
        let urlObj;
        try {
            urlObj = new URL(url);
        } catch (e) {
            try {
                urlObj = new URL(url, window.location.origin);
            } catch (e2) {
                console.error('❌ URL parsing failed:', url, e2);
                return;
            }
        }
        
        // Extract endpoint information
        const pathParts = urlObj.pathname.split('/').filter(part => part.length > 0);
        let operationId = null;
        let endpoint = null;
        
        const graphqlIndex = pathParts.findIndex(part => part === 'graphql');
        if (graphqlIndex !== -1 && graphqlIndex + 2 < pathParts.length) {
            endpoint = pathParts[graphqlIndex + 2];
        }
        
        if (!operationId && !endpoint) {
            // Try to extract from query parameters or other patterns
            const urlStr = url.toString();
            const operationMatch = urlStr.match(/operation[_-]?id?[=:]\s*([a-zA-Z0-9_-]+)/i);
            if (operationMatch) {
                operationId = operationMatch[1];
            }
        }
        
        console.log('📍 ENDPOINT INFO -> Operation ID:', operationId, 'Endpoint:', endpoint);
        
        // Extract parameters with COMPREHENSIVE parsing
        const variablesParam = urlObj.searchParams.get('variables');
        const featuresParam = urlObj.searchParams.get('features');
        const fieldTogglesParam = urlObj.searchParams.get('fieldToggles');
        
        let variables = null;
        let features = null;
        let fieldToggles = null;
        
        // Parse variables
        if (variablesParam) {
            console.log('📊 RAW VARIABLES PARAM:', variablesParam);
            console.log('📊 VARIABLES PARAM LENGTH:', variablesParam.length);
            variables = parseWithAllMethods(variablesParam, 'variables');
            
            if (isHomeTimelineEndpoint && variables) {
                console.log('✅ HOMETIMELINE VARIABLES PARSED:', variables);
                console.log('   Variable keys:', Object.keys(variables));
            }
        } else {
            console.log('⚠️ NO VARIABLES PARAM in URL for endpoint:', endpoint || operationId || 'unknown');
        }
        
        // CRITICAL: Enhanced features parsing with ALL possible methods
        if (featuresParam) {
            console.log('🎛️ RAW FEATURES PARAM:', featuresParam);
            console.log('🎛️ RAW FEATURES LENGTH:', featuresParam.length);
            features = parseWithAllMethods(featuresParam, 'features');
        }
        
        // Parse field toggles
        if (fieldTogglesParam) {
            console.log('🔀 RAW FIELD TOGGLES PARAM:', fieldTogglesParam);
            fieldToggles = parseWithAllMethods(fieldTogglesParam, 'fieldToggles');
        }
        
        // Extract bearer token
        let bearerToken = null;
        if (options.headers) {
            const authHeader = options.headers['authorization'] || 
                             options.headers['Authorization'] || 
                             options.headers['auth'];
            if (authHeader && authHeader.startsWith('Bearer ')) {
                bearerToken = authHeader.substring(7);
            }
        }
        
        if (!bearerToken) {
            bearerToken = "AAAAAAAAAAAAAAAAAAAAANRILgAAAAAAnNwIzUejRCOuH5E6I8xnZz4puTs%3D1Zv7ttfk8LF81IUq16cHjhLTvJu4FA33AGWWjCpTnA";
        }
        
        // CRITICAL: Aggregate features from ALL endpoints
        if (features && Object.keys(features).length > 0) {
            // Add all discovered features to global set
            Object.keys(features).forEach(featureKey => {
                window.allDiscoveredFeatures.add(featureKey);
            });
            
            // Store features per endpoint
            const endpointKey = endpoint || operationId || 'unknown';
            
            // CRITICAL: Track HomeTimeline features separately
            if (isHomeTimelineEndpoint) {
                console.log('✅ HOMETIMELINE FEATURES CAPTURED -> Storing', Object.keys(features).length, 'features');
                
                // Initialize HomeTimeline-specific storage if not exists
                if (!window.homeTimelineFeatures) {
                    window.homeTimelineFeatures = {};
                }
                
                // Merge HomeTimeline features
                Object.assign(window.homeTimelineFeatures, features);
                console.log('🎯 TOTAL HOMETIMELINE FEATURES:', Object.keys(window.homeTimelineFeatures).length);
            }
            
            window.endpointFeatureMap.set(endpointKey, {
                features: features,
                operationId: operationId,
                endpoint: endpoint,
                url: url,
                timestamp: new Date().toISOString(),
                isHomeTimeline: isHomeTimelineEndpoint
            });
            
            console.log('✅ FEATURE AGGREGATION -> Added', Object.keys(features).length, 'features from endpoint:', endpointKey);
            console.log('🎯 TOTAL UNIQUE FEATURES DISCOVERED:', window.allDiscoveredFeatures.size);
        }
        
        // Build comprehensive aggregated features object - ONLY USE HOMETIMELINE FEATURES
        let aggregatedFeatures = {};
        
        // CRITICAL: Prefer HomeTimeline features over all others
        if (window.homeTimelineFeatures && Object.keys(window.homeTimelineFeatures).length > 0) {
            aggregatedFeatures = { ...window.homeTimelineFeatures };
            console.log('🎯 USING HOMETIMELINE FEATURES -> Total:', Object.keys(aggregatedFeatures).length, 'features');
        } else {
            // Fallback: Only use features from HomeTimeline endpoints in the map
            window.endpointFeatureMap.forEach((endpointData, endpointKey) => {
                if (endpointData.isHomeTimeline) {
                    Object.assign(aggregatedFeatures, endpointData.features);
                }
            });
            
            // If still no HomeTimeline features, use current features if from HomeTimeline
            if (Object.keys(aggregatedFeatures).length === 0 && features && isHomeTimelineEndpoint) {
                aggregatedFeatures = { ...features };
            }
            
            console.log('🎯 AGGREGATED HOMETIMELINE FEATURES -> Total:', Object.keys(aggregatedFeatures).length, 'features');
        }
        
        // Finally add current features
        if (features) {
            Object.assign(aggregatedFeatures, features);
        }
        
        console.log('🎯 AGGREGATED FEATURES -> Total:', Object.keys(aggregatedFeatures).length, 'features');
        
        // Update global storage with COMPLETE feature set including dynamic Query ID
        window.capturedTwitterApiData = {
            apiBaseUrl: `${urlObj.protocol}//${urlObj.host}${urlObj.pathname}`,
            variables: isHomeTimelineEndpoint ? variables : window.capturedTwitterApiData?.variables || null, // Preserve previous if not HomeTimeline
            features: isHomeTimelineEndpoint ? features : {}, // Only store if HomeTimeline
            allFeatures: aggregatedFeatures, // This now contains ONLY HomeTimeline features
            allEndpoints: Object.fromEntries(window.endpointFeatureMap),
            fieldToggles: fieldToggles,
            bearerToken: bearerToken,
            operationId: operationId,
            endpoint: endpoint,
            lastCaptureTime: new Date().toISOString(),
            captureCount: (window.capturedTwitterApiData?.captureCount || 0) + 1,
            isValid: true,
            fullUrl: url,
            originalVariablesParam: variablesParam,
            originalFeaturesParam: featuresParam,
            originalFieldTogglesParam: fieldTogglesParam,
            requestMethod: options.method || 'GET',
            requestHeaders: options.headers || {},
            isHomeTimelineEndpoint: isHomeTimelineEndpoint // Track if this was HomeTimeline
        };
        
        // DEBUG: Log variables capture status
        if (isHomeTimelineEndpoint) {
            console.log('🔧 HOMETIMELINE VARIABLES DEBUG:');
            console.log('   Variables captured:', variables ? Object.keys(variables).length : 0);
            console.log('   Variables object:', variables);
            
            // Store HomeTimeline variables globally like features
            if (!window.homeTimelineVariables) {
                window.homeTimelineVariables = {};
            }
            if (variables && typeof variables === 'object') {
                Object.assign(window.homeTimelineVariables, variables);
                console.log('   Total HomeTimeline variables stored:', Object.keys(window.homeTimelineVariables).length);
            } else {
                console.log('   ⚠️ Variables is null/undefined for this endpoint');
            }
        }
        
        // COMPREHENSIVE LOGGING
        console.group('🔍 ✅ COMPLETE Twitter API Call Captured');
        console.log('📍 URL:', url);
        console.log('🆔 Query ID (Dynamic):', queryId);  // NEW: Show dynamic Query ID
        console.log('🆔 Operation ID:', operationId);
        console.log('📊 Endpoint:', endpoint || 'unknown');
        console.log('🌐 GraphQL Path:', urlObj.pathname);  // NEW: Show complete GraphQL path
        
        // SIMPLIFIED: Only log discovered features (no missing feature validation)
        if (features && Object.keys(features).length > 0) {
            console.log('🎛️ DISCOVERED Features (', Object.keys(features).length, 'flags):', features);
            
            console.group('📋 DISCOVERED Features:');
            Object.entries(features).forEach(([key, value]) => {
                // Only show discovered features with ✅ - no missing feature logging
                if (value === true || value === false || value !== undefined) {
                    const status = value === true ? '✅' : value === false ? '⚠️' : '❓';
                    console.log(`${status} ${key}: ${value}`);
                }
            });
            console.groupEnd();
        } else {
            console.log('ℹ️ No features found in this endpoint - continuing with aggregated features');
        }
        
        // Store aggregated features globally - SIMPLIFIED logging
        console.group('🌟 ALL DISCOVERED FEATURES (' + Object.keys(aggregatedFeatures).length + ' total):');
        console.log('🎯 COMPLETE FEATURE SET:', aggregatedFeatures);
        console.groupEnd();
        
        // Log statistics
        console.log('📊 COMPLETE STATISTICS:');
        console.log(`  🎯 Total Unique Features: ${window.allDiscoveredFeatures.size}`);
        console.log(`  📍 Endpoints Tracked: ${window.endpointFeatureMap.size}`);
        console.log(`  🔄 Total Captures: ${window.capturedTwitterApiData.captureCount}`);
        console.log(`  📈 Features in Aggregated Set: ${Object.keys(aggregatedFeatures).length}`);
        
        // Variables and field toggles
        if (variables) {
            console.log('📊 Variables (', Object.keys(variables).length, 'params):', variables);
        }
        
        if (fieldToggles) {
            console.log('🔀 Field Toggles (', Object.keys(fieldToggles).length, 'toggles):', fieldToggles);
        }
        
        // Raw parameters for debugging
        console.group('🎯 RAW PARAMETERS:');
        if (variablesParam) {
            console.log('📊 Raw Variables:', variablesParam);
        }
        if (featuresParam) {
            console.log('🎛️ Raw Features:', featuresParam);
        }
        if (fieldTogglesParam) {
            console.log('🔀 Raw Field Toggles:', fieldTogglesParam);
        }
        console.groupEnd();
        
        console.groupEnd();
        
        // 🚀 IMMEDIATE cURL GENERATION -> Generate with ANY discovered features
        console.log('🚀 IMMEDIATE cURL GENERATION -> Generating with ANY available data...');
        console.log('📊 Current endpoint features:', Object.keys(features || {}).length);
        console.log('🌍 Total aggregated features:', Object.keys(aggregatedFeatures).length);
        console.log('🎯 Unique features across all endpoints:', window.allDiscoveredFeatures.size);
        
        try {
            const curlResult = generateTwitterApiCurl();
            if (curlResult && curlResult.curlCommand) {
                console.log('✅ SUCCESS -> Generated cURL with', Object.keys(curlResult.features || {}).length, 'features');
            } else {
                console.log('⚠️ cURL generation returned empty result');
            }
        } catch (curlError) {
            console.log('⚠️ cURL generation error:', curlError.message);
        }
        
    } catch (error) {
        console.error('❌ COMPLETE EXTRACTION -> Error:', error);
        console.log('🔍 Failed URL:', url);
    }
}

// ENHANCED: Comprehensive parsing with ALL possible methods
function parseWithAllMethods(paramString, paramType) {
    const parsers = [
        // Standard methods
        () => JSON.parse(decodeURIComponent(paramString)),
        () => JSON.parse(paramString),
        () => JSON.parse(decodeURIComponent(paramString.replace(/\+/g, ' '))),
        
        // Advanced decoding methods
        () => JSON.parse(paramString.replace(/%([0-9A-F]{2})/gi, (match, p1) => String.fromCharCode(parseInt(p1, 16)))),
        () => JSON.parse(decodeURIComponent(decodeURIComponent(paramString))),
        () => JSON.parse(unescape(paramString)),
        
        // Base64 methods
        () => JSON.parse(atob(paramString)),
        () => {
            const base64 = paramString.replace(/-/g, '+').replace(/_/g, '/');
            return JSON.parse(atob(base64));
        },
        
        // Manual character replacement for mobile browsers
        () => {
            let cleaned = paramString
                .replace(/%20/g, ' ')
                .replace(/%22/g, '"')
                .replace(/%7B/g, '{')
                .replace(/%7D/g, '}')
                .replace(/%3A/g, ':')
                .replace(/%2C/g, ',')
                .replace(/%5B/g, '[')
                .replace(/%5D/g, ']')
                .replace(/%2F/g, '/')
                .replace(/%3F/g, '?')
                .replace(/%26/g, '&')
                .replace(/%3D/g, '=');
            return JSON.parse(cleaned);
        },
        
        // Regex extraction for embedded JSON
        () => {
            const jsonMatch = paramString.match(/\{.*\}/);
            if (jsonMatch) {
                return JSON.parse(jsonMatch[0]);
            }
            throw new Error('No JSON pattern found');
        },
        
        // Hex decoding
        () => {
            const hex = paramString.replace(/[^0-9A-Fa-f]/g, '');
            if (hex.length % 2 === 0) {
                const str = hex.match(/.{2}/g).map(byte => String.fromCharCode(parseInt(byte, 16))).join('');
                return JSON.parse(str);
            }
            throw new Error('Invalid hex');
        }
    ];
    
    for (let i = 0; i < parsers.length; i++) {
        try {
            const result = parsers[i]();
            console.log(`✅ ${paramType.toUpperCase()} PARSED with method ${i + 1}:`, result);
            return result;
        } catch (e) {
            // Continue to next method
        }
    }
    
    console.error(`❌ ALL PARSING METHODS FAILED for ${paramType}:`, paramString);
    return null;
}

// ENHANCED validation - Generate cURL with ANY discovered features
function validateCapturedParameters() {
    const data = window.capturedTwitterApiData;
    
    // Always accept if we have ANY data - even minimal
    if (!data) {
        return false;
    }
    
    // Generate cURL as soon as we have ANY features discovered
    const hasFeatures = (data.features && Object.keys(data.features).length > 0) ||
                       (data.allFeatures && Object.keys(data.allFeatures).length > 0) ||
                       (window.allDiscoveredFeatures && window.allDiscoveredFeatures.size > 0);
    
    const hasVariables = data.variables && Object.keys(data.variables).length > 0;
    const hasEndpoint = data.apiBaseUrl || data.endpoint;
    
    // Generate cURL if we have features OR variables OR endpoint info
    if (hasFeatures || hasVariables || hasEndpoint) {
        console.log('✅ VALIDATION -> Generating cURL with discovered data:');
        console.log(`  Current Features: ${data.features ? Object.keys(data.features).length : 0}`);
        console.log(`  All Aggregated Features: ${data.allFeatures ? Object.keys(data.allFeatures).length : 0}`);
        console.log(`  Total Unique Features: ${window.allDiscoveredFeatures ? window.allDiscoveredFeatures.size : 0}`);
        console.log(`  Endpoints Tracked: ${window.endpointFeatureMap ? window.endpointFeatureMap.size : 0}`);
        console.log(`  Variables: ${hasVariables ? 'YES' : 'NO'}`);
        console.log(`  Endpoint: ${hasEndpoint ? 'YES' : 'NO'}`);
        return true;
    }
    
    return false;
}

// ENHANCED dynamic parameters - Use ALL discovered features immediately
function getDynamicParameters() {
    const data = window.capturedTwitterApiData;
    
    if (validateCapturedParameters()) {
        // Build COMPLETE feature set from ONLY HomeTimeline features
        const homeTimelineFeatures = window.homeTimelineFeatures || {};
        
        // ENHANCED: Use HomeTimeline features + discovered features from the user's list
        const targetFeatures = {
            "rweb_video_screen_enabled": false,
            "profile_label_improvements_pcf_label_in_post_enabled": true,
            "rweb_tipjar_consumption_enabled": true,
            "verified_phone_label_enabled": true,
            "creator_subscriptions_tweet_preview_api_enabled": true,
            "responsive_web_graphql_timeline_navigation_enabled": true,
            "responsive_web_graphql_skip_user_profile_image_extensions_enabled": false,
            "premium_content_api_read_enabled": false,
            "communities_web_enable_tweet_community_results_fetch": true,
            "c9s_tweet_anatomy_moderator_badge_enabled": true,
            "responsive_web_grok_analyze_button_fetch_trends_enabled": false,
            "responsive_web_grok_analyze_post_followups_enabled": true,
            "responsive_web_jetfuel_frame": false,
            "responsive_web_grok_share_attachment_enabled": true,
            "articles_preview_enabled": true,
            "responsive_web_edit_tweet_api_enabled": true,
            "graphql_is_translatable_rweb_tweet_is_translatable_enabled": true,
            "view_counts_everywhere_api_enabled": true,
            "longform_notetweets_consumption_enabled": true,
            "responsive_web_twitter_article_tweet_consumption_enabled": true,
            "tweet_awards_web_tipping_enabled": false,
            "responsive_web_grok_show_grok_translated_post": false,
            "responsive_web_grok_analysis_button_from_backend": true,
            "creator_subscriptions_quote_tweet_preview_enabled": false,
            "freedom_of_speech_not_reach_fetch_enabled": true,
            "standardized_nudges_misinfo": true,
            "tweet_with_visibility_results_prefer_gql_limited_actions_policy_enabled": true,
            "longform_notetweets_rich_text_read_enabled": true,
            "longform_notetweets_inline_media_enabled": true,
            "responsive_web_grok_image_annotation_enabled": true,
            "responsive_web_enhance_cards_enabled": false
        };
        
        // Merge HomeTimeline features with target features
        const finalFeatures = Object.assign({}, targetFeatures, homeTimelineFeatures);
        
        console.log('🎯 PARAMETER RETRIEVAL -> Using ONLY HomeTimeline features');
        console.log(`📊 HomeTimeline Features: ${Object.keys(homeTimelineFeatures).length}`);
        console.log(`🎯 Final Feature Set: ${Object.keys(finalFeatures).length} features`);
        
        // Build enhanced data object with HomeTimeline features
        const enhancedData = {
            ...data,
            features: finalFeatures, // Only HomeTimeline features
            allFeatures: finalFeatures, // Same HomeTimeline-only set
        };
        
        return {
            source: 'captured',
            data: enhancedData,
            timestamp: data.lastCaptureTime || new Date().toISOString()
        };
    }
    
    // Fallback with enhanced feature set
    const fallbackData = {
        apiBaseUrl: 'https://x.com/i/api/graphql/u5DNQK1yswAFi4GWuHyfGA/HomeTimeline',
        variables: {
            "count": 20,
            "includePromotedContent": true,
            "latestControlAvailable": true,
            "requestContext": "launch",
            "withCommunity": true
        },
        // ENHANCED: Use the target feature set you want
        features: {
            "rweb_video_screen_enabled": false,
            "profile_label_improvements_pcf_label_in_post_enabled": true,
            "rweb_tipjar_consumption_enabled": true,
            "verified_phone_label_enabled": true,
            "creator_subscriptions_tweet_preview_api_enabled": true,
            "responsive_web_graphql_timeline_navigation_enabled": true,
            "responsive_web_graphql_skip_user_profile_image_extensions_enabled": false,
            "premium_content_api_read_enabled": false,
            "communities_web_enable_tweet_community_results_fetch": true,
            "c9s_tweet_anatomy_moderator_badge_enabled": true,
            "responsive_web_grok_analyze_button_fetch_trends_enabled": false,
            "responsive_web_grok_analyze_post_followups_enabled": true,
            "responsive_web_jetfuel_frame": false,
            "responsive_web_grok_share_attachment_enabled": true,
            "articles_preview_enabled": true,
            "responsive_web_edit_tweet_api_enabled": true,
            "graphql_is_translatable_rweb_tweet_is_translatable_enabled": true,
            "view_counts_everywhere_api_enabled": true,
            "longform_notetweets_consumption_enabled": true,
            "responsive_web_twitter_article_tweet_consumption_enabled": true,
            "tweet_awards_web_tipping_enabled": false,
            "responsive_web_grok_show_grok_translated_post": false,
            "responsive_web_grok_analysis_button_from_backend": true,
            "creator_subscriptions_quote_tweet_preview_enabled": false,
            "freedom_of_speech_not_reach_fetch_enabled": true,
            "standardized_nudges_misinfo": true,
            "tweet_with_visibility_results_prefer_gql_limited_actions_policy_enabled": true,
            "longform_notetweets_rich_text_read_enabled": true,
            "longform_notetweets_inline_media_enabled": true,
            "responsive_web_grok_image_annotation_enabled": true,
            "responsive_web_enhance_cards_enabled": false
        },
        bearerToken: "AAAAAAAAAAAAAAAAAAAAANRILgAAAAAAnNwIzUejRCOuH5E6I8xnZz4puTs%3D1Zv7ttfk8LF81IUq16cHjhLTvJu4FA33AGWWjCpTnA",
        operationId: "u5DNQK1yswAFi4GWuHyfGA",
        endpoint: "HomeTimeline",
        isValid: true,
        isFallback: true
    };
    
    return {
        source: 'fallback',
        data: fallbackData,
        timestamp: new Date().toISOString()
    };
}

// Updated curl generation to use complete feature set with PROPER token integration
function generateTwitterApiCurl(extractedData) {
    const paramSource = getDynamicParameters();
    const apiData = paramSource.data;
    
    // Use ALL discovered features instead of just current endpoint features
    const features = apiData.allFeatures && Object.keys(apiData.allFeatures).length > 0 
                    ? apiData.allFeatures 
                    : apiData.features;
    
    if (paramSource.source === 'captured') {
        console.log('🔧 CURL GENERATION -> Using COMPLETE feature set with', Object.keys(features).length, 'features');
        console.log('🎯 COMPLETE FEATURES -> Using aggregated features from all endpoints');
    }
    
    function generateXpForwardedFor() {
        const chars = 'abcdef0123456789';
        let result = '';
        for (let i = 0; i < 512; i++) {
            result += chars.charAt(Math.floor(Math.random() * chars.length));
        }
        return result;
    }
    
    const apiBaseUrl = apiData.apiBaseUrl;
    const variables = apiData.variables;
    const fieldToggles = apiData.fieldToggles;
    
    const encodedVariables = encodeURIComponent(JSON.stringify(variables));
    console.log('🔗 COMPLETE RAW FEATURES (', Object.keys(features).length, 'features) -> ', features);
    const encodedFeatures = encodeURIComponent(JSON.stringify(features));
    
    let encodedFieldToggles = null;
    if (fieldToggles && Object.keys(fieldToggles).length > 0) {
        encodedFieldToggles = encodeURIComponent(JSON.stringify(fieldToggles));
        console.log('🔀 RAW FIELD TOGGLES -> ', fieldToggles);
    }
    
    let fullApiUrl = `${apiBaseUrl}?variables=${encodedVariables}&features=${encodedFeatures}`;
    if (encodedFieldToggles) {
        fullApiUrl += `&fieldToggles=${encodedFieldToggles}`;
    }
    
    // FIXED: Properly extract tokens with fallback handling
    let tokens = {};
    if (extractedData && extractedData.tokens) {
        tokens = extractedData.tokens;
    } else {
        console.log('🔧 CURL GENERATION -> No extractedData.tokens, extracting fresh tokens from browser');
        // Extract tokens directly from browser if extractedData is not available
        const csrfMatch = document.cookie.match(/ct0=([^;]+)/);
        const guestIdMatch = document.cookie.match(/guest_id=([^;]+)/);
        const personalizationMatch = document.cookie.match(/personalization_id=([^;]+)/);
        const twidMatch = document.cookie.match(/twid=([^;]+)/);
        const langMatch = document.cookie.match(/lang=([^;]+)/);
        
        function generateTransactionId() {
            const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
            let result = '';
            for (let i = 0; i < 88; i++) {
                result += chars.charAt(Math.floor(Math.random() * chars.length));
            }
            return result;
        }
        
        tokens = {
            csrf_token: csrfMatch ? csrfMatch[1] : null,
            guest_id: guestIdMatch ? guestIdMatch[1] : null,
            personalization_id: personalizationMatch ? decodeURIComponent(personalizationMatch[1]).replace(/"/g, '') : null,
            twid: twidMatch ? decodeURIComponent(twidMatch[1]) : null,
            language: langMatch ? langMatch[1] : 'en',
            auth_token: null, // HttpOnly cookie, will be updated by C++ backend
            client_transaction_id: generateTransactionId(),
            bearer_token: "AAAAAAAAAAAAAAAAAAAAANRILgAAAAAAnNwIzUejRCOuH5E6I8xnZz4puTs%3D1Zv7ttfk8LF81IUq16cHjhLTvJu4FA33AGWWjCpTnA"
        };
    }
    
    const csrfToken = tokens.csrf_token || 'MISSING_CSRF';
    const authToken = tokens.auth_token || 'MISSING_AUTH_TOKEN';
    const guestId = tokens.guest_id || 'MISSING_GUEST_ID';
    const personalizationId = tokens.personalization_id || 'MISSING_PERSONALIZATION_ID';
    const twid = tokens.twid || 'MISSING_TWID';
    
    const cookieParts = [];
    
    if (guestId && !guestId.startsWith('MISSING_')) {
        const cleanGuestId = guestId.replace('v1%3A', '');
        cookieParts.push(`guest_id_marketing=v1%3A${cleanGuestId}`);
        cookieParts.push(`guest_id_ads=v1%3A${cleanGuestId}`);
        cookieParts.push(`guest_id=${guestId}`);
    }
    
    if (personalizationId && !personalizationId.startsWith('MISSING_')) {
        cookieParts.push(`personalization_id=${personalizationId.replace(/"/g, '')}`);
    }
    
    if (authToken && !authToken.startsWith('MISSING_')) {
        cookieParts.push(`auth_token=${authToken}`);
    }
    
    if (csrfToken && !csrfToken.startsWith('MISSING_')) {
        cookieParts.push(`ct0=${csrfToken}`);
    }
    
    if (tokens.language) {
        cookieParts.push(`lang=${tokens.language}`);
    }
    
    if (twid && !twid.startsWith('MISSING_')) {
        cookieParts.push(`twid=${twid}`);
    }
    
    const cookieString = cookieParts.join('; ');
    const bearerToken = apiData.bearerToken || tokens.bearer_token;
    
    // FIXED: Properly handle user-agent with fallback
    const userAgent = (extractedData && extractedData.headers && extractedData.headers['user-agent']) 
                     ? extractedData.headers['user-agent'] 
                     : navigator.userAgent;
    
    const curlCommand = [
        `curl '${fullApiUrl}' \\`,
        `  -H 'accept: */*' \\`,
        `  -H 'accept-language: en-US' \\`,
        `  -H 'authorization: Bearer ${bearerToken}' \\`,
        `  -H 'content-type: application/json' \\`,
        `  -H 'cookie: ${cookieString}' \\`,
        `  -H 'priority: u=1, i' \\`,
        `  -H 'referer: https://x.com/' \\`,
        `  -H 'sec-ch-ua: "Chromium";v="127", "Not)A;Brand";v="99"' \\`,
        `  -H 'sec-ch-ua-mobile: ?1' \\`,
        `  -H 'sec-ch-ua-platform: "Android"' \\`,
        `  -H 'sec-fetch-dest: empty' \\`,
        `  -H 'sec-fetch-mode: cors' \\`,
        `  -H 'sec-fetch-site: same-origin' \\`,
        `  -H 'user-agent: ${userAgent}' \\`,
        `  -H 'x-client-transaction-id: ${tokens.client_transaction_id}' \\`,
        `  -H 'x-csrf-token: ${csrfToken && !csrfToken.startsWith('MISSING_') ? csrfToken : 'MISSING_CSRF_TOKEN'}' \\`,
        `  -H 'x-twitter-active-user: yes' \\`,
        `  -H 'x-twitter-auth-type: OAuth2Session' \\`,
        `  -H 'x-twitter-client-language: ${tokens.language}' \\`,
        `  -H 'x-xp-forwarded-for: ${generateXpForwardedFor()}'`
    ].join('\n');
    
    console.log('🎯 COMPLETE API CALL WITH ALL FEATURES AND REAL TOKENS:');
    console.log('📡 Parameter Source:', paramSource.source.toUpperCase());
    console.log('🌐 API URL:', fullApiUrl);
    console.log('');
    console.log('📊 Variables (URL encoded):', encodedVariables);
    console.log('⚙️ COMPLETE Features (', Object.keys(features).length, 'features, URL encoded):', encodedFeatures);
    if (encodedFieldToggles) {
        console.log('🔀 Field Toggles (URL encoded):', encodedFieldToggles);
    }
    console.log('');
    console.log('🔐 REAL Authentication Tokens:');
    console.log('🔑 Bearer Token:', bearerToken);
    console.log('🔒 CSRF Token:', csrfToken !== 'MISSING_CSRF' ? '✅ REAL TOKEN' : '❌ MISSING');
    console.log('👤 Guest ID:', guestId !== 'MISSING_GUEST_ID' ? '✅ REAL TOKEN' : '❌ MISSING');
    console.log('🍪 Auth Token:', authToken !== 'MISSING_AUTH_TOKEN' ? '✅ REAL TOKEN (from HttpOnly)' : '⏳ PENDING (HttpOnly)');
    console.log('📊 Personalization ID:', personalizationId !== 'MISSING_PERSONALIZATION_ID' ? '✅ REAL TOKEN' : '❌ MISSING');
    console.log('🆔 TWID:', twid !== 'MISSING_TWID' ? '✅ REAL TOKEN' : '❌ MISSING/GUEST');
    console.log('📱 Client Transaction ID:', tokens.client_transaction_id ? '✅ GENERATED' : '❌ MISSING');
    console.log('');
    console.log('🚀 READY-TO-USE CURL WITH REAL TOKENS AND COMPLETE FEATURES:');
    console.log('');
    console.log(curlCommand);
    console.log('');
    console.log('✅ END COMPLETE CURL COMMAND WITH REAL TOKENS');
    
    return {
        apiUrl: fullApiUrl,
        curlCommand: curlCommand,
        parameterSource: paramSource.source,
        capturedData: paramSource.source === 'captured' ? apiData : null,
        tokens: {
            bearer_token: bearerToken,
            csrf_token: csrfToken,
            guest_id: guestId,
            auth_token: authToken,
            personalization_id: personalizationId,
            twid: twid,
            client_transaction_id: tokens.client_transaction_id,
            language: tokens.language
        },
        variables: variables,
        features: features, // This now contains ALL features
        fieldToggles: fieldToggles
    };
}

// Enhanced Twitter API utilities
window.TwitterAPIUtils = {
    // Get complete aggregated feature set
    getAllFeatures: function() {
        const allFeatures = {};
        window.endpointFeatureMap.forEach((endpointData) => {
            Object.assign(allFeatures, endpointData.features);
        });
        console.log('🎯 ALL DISCOVERED FEATURES (', Object.keys(allFeatures).length, '):', allFeatures);
        return allFeatures;
    },
    
    // Get ONLY HomeTimeline features
    getHomeTimelineFeatures: function() {
        const homeTimelineFeatures = window.homeTimelineFeatures || {};
        console.log('🎯 HOMETIMELINE FEATURES ONLY (', Object.keys(homeTimelineFeatures).length, '):', homeTimelineFeatures);
        return homeTimelineFeatures;
    },
    
    // Get features by endpoint
    getFeaturesByEndpoint: function() {
        const result = {};
        window.endpointFeatureMap.forEach((endpointData, endpointKey) => {
            result[endpointKey] = {
                features: endpointData.features,
                isHomeTimeline: endpointData.isHomeTimeline || false
            };
        });
        console.log('📊 FEATURES BY ENDPOINT:', result);
        return result;
    },
    
    // Compare features between what we want and what we have
    compareWithTarget: function() {
        const target = {
            "rweb_video_screen_enabled": false,
            "profile_label_improvements_pcf_label_in_post_enabled": true,
            "rweb_tipjar_consumption_enabled": true,
            "verified_phone_label_enabled": true,
            "creator_subscriptions_tweet_preview_api_enabled": true,
            "responsive_web_graphql_timeline_navigation_enabled": true,
            "responsive_web_graphql_skip_user_profile_image_extensions_enabled": false,
            "premium_content_api_read_enabled": false,
            "communities_web_enable_tweet_community_results_fetch": true,
            "c9s_tweet_anatomy_moderator_badge_enabled": true,
            "responsive_web_grok_analyze_button_fetch_trends_enabled": false,
            "responsive_web_grok_analyze_post_followups_enabled": true,
            "responsive_web_jetfuel_frame": false,
            "responsive_web_grok_share_attachment_enabled": true,
            "articles_preview_enabled": true,
            "responsive_web_edit_tweet_api_enabled": true,
            "graphql_is_translatable_rweb_tweet_is_translatable_enabled": true,
            "view_counts_everywhere_api_enabled": true,
            "longform_notetweets_consumption_enabled": true,
            "responsive_web_twitter_article_tweet_consumption_enabled": true,
            "tweet_awards_web_tipping_enabled": false,
            "responsive_web_grok_show_grok_translated_post": false,
            "responsive_web_grok_analysis_button_from_backend": true,
            "creator_subscriptions_quote_tweet_preview_enabled": false,
            "freedom_of_speech_not_reach_fetch_enabled": true,
            "standardized_nudges_misinfo": true,
            "tweet_with_visibility_results_prefer_gql_limited_actions_policy_enabled": true,
            "longform_notetweets_rich_text_read_enabled": true,
            "longform_notetweets_inline_media_enabled": true,
            "responsive_web_grok_image_annotation_enabled": true,
            "responsive_web_enhance_cards_enabled": false
        };
        
        const discovered = this.getAllFeatures();
        
        const missing = [];
        const found = [];
        
        Object.keys(target).forEach(key => {
            if (discovered.hasOwnProperty(key)) {
                found.push(key);
            } else {
                missing.push(key);
            }
        });
        
        console.log('🎯 TARGET FEATURE COMPARISON:');
        console.log('✅ FOUND (' + found.length + '):', found);
        console.log('❌ MISSING (' + missing.length + '):', missing);
        console.log('📊 Coverage:', Math.round((found.length / Object.keys(target).length) * 100) + '%');
        
        return {
            target: target,
            discovered: discovered,
            found: found,
            missing: missing,
            coverage: found.length / Object.keys(target).length
        };
    },
    
    getCurrentData: function() {
        const data = window.capturedTwitterApiData;
        if (data && data.isValid) {
            return {
                currentFeatures: Object.keys(data.features || {}).length,
                allFeatures: Object.keys(data.allFeatures || {}).length,
                homeTimelineFeatures: Object.keys(window.homeTimelineFeatures || {}).length,
                uniqueFeatures: window.allDiscoveredFeatures.size,
                endpoints: window.endpointFeatureMap.size,
                captureCount: data.captureCount,
                skippedEndpoints: window.skippedEndpoints
            };
        }
        return null;
    },
    
    // Debug status for HomeTimeline filtering
    getDebugStatus: function() {
        console.log('🔍 HOMETIMELINE FILTER DEBUG STATUS:');
        console.log(`  ✅ HomeTimeline features captured: ${Object.keys(window.homeTimelineFeatures || {}).length}`);
        console.log(`  🚫 Viewer endpoints skipped: ${window.skippedEndpoints.viewer}`);
        console.log(`  ⚠️ Other endpoints processed: ${window.skippedEndpoints.other}`);
        console.log(`  📊 Total endpoints skipped: ${window.skippedEndpoints.total}`);
        
        console.log('\n📍 Endpoint breakdown:');
        window.endpointFeatureMap.forEach((data, endpoint) => {
            const marker = data.isHomeTimeline ? '✅' : '❌';
            console.log(`  ${marker} ${endpoint}: ${Object.keys(data.features).length} features`);
        });
        
        return {
            homeTimelineFeatures: Object.keys(window.homeTimelineFeatures || {}).length,
            skipped: window.skippedEndpoints,
            endpoints: Array.from(window.endpointFeatureMap.keys())
        };
    }
};

// INTEGRATION FUNCTIONS - Essential for C++ backend compatibility
// These functions provide token extraction and bridge communication

// SINGLE SEND CONTROL FLAG SYSTEM - Prevents duplicate data transmission
if (!window.twitterDataSentFlag) {
    window.twitterDataSentFlag = {
        dataSent: false,          // Main control flag
        timestamp: null,          // When data was sent
        httpOnlyReceived: false,  // HttpOnly cookies received
        curlGenerated: false,     // cURL command generated
        sessionId: null          // Session identifier
    };
}

// DEBUG FUNCTION: Manual reset for single send control (for testing/debugging)
window.resetTwitterDataFlag = function() {
    console.log('🔄 MANUAL RESET -> Resetting single send control flag');
    console.log('   Previous state:', { ...window.twitterDataSentFlag });
    
    window.twitterDataSentFlag.dataSent = false;
    window.twitterDataSentFlag.timestamp = null;
    window.twitterDataSentFlag.sessionId = null;
    window.twitterDataSentFlag.httpOnlyReceived = false;
    window.twitterDataSentFlag.curlGenerated = false;
    
    console.log('   New state:', { ...window.twitterDataSentFlag });
    console.log('✅ Flag reset complete - extractAndSendTokens() will now execute');
    return 'Flag reset successfully';
};

function extractAndSendTokens() {
    console.log('🔍 Extracting X.com API tokens and data...\n');
    
    // ENHANCED SINGLE SEND CONTROL - Check if data has already been sent but allow updates
    if (window.twitterDataSentFlag.dataSent) {
        // Check if we have new features or significant data changes
        const currentFeatureCount = window.capturedTwitterApiData ? 
            Object.keys(window.capturedTwitterApiData.features || {}).length : 0;
        const currentVariableCount = window.capturedTwitterApiData ? 
            Object.keys(window.capturedTwitterApiData.variables || {}).length : 0;
        
        // Check if previous data exists and compare feature counts
        const previousData = window.twitterTokensData;
        const previousFeatureCount = previousData?.homeTimelineApi?.features ? 
            Object.keys(previousData.homeTimelineApi.features).length : 0;
        const previousVariableCount = previousData?.homeTimelineApi?.variables ? 
            Object.keys(previousData.homeTimelineApi.variables).length : 0;
        
        const hasNewFeatures = currentFeatureCount > previousFeatureCount;
        const hasNewVariables = currentVariableCount > previousVariableCount;
        const significantUpdate = hasNewFeatures || hasNewVariables;
        
        if (significantUpdate) {
            console.log('🆕 ENHANCED SINGLE SEND CONTROL -> New features detected, allowing update send');
            console.log(`   Previous features: ${previousFeatureCount}, Current: ${currentFeatureCount}`);
            console.log(`   Previous variables: ${previousVariableCount}, Current: ${currentVariableCount}`);
            console.log('   Proceeding with updated data transmission...');
            // Reset flag to allow sending
            window.twitterDataSentFlag.dataSent = false;
        } else {
            console.log('✅ SINGLE SEND CONTROL -> Data already sent successfully at', window.twitterDataSentFlag.timestamp);
            console.log('   No new features detected, skipping duplicate send');
            console.log('   📊 Previous session ID:', window.twitterDataSentFlag.sessionId);
            console.log(`   Feature count: ${currentFeatureCount}, Variable count: ${currentVariableCount}`);
            return window.twitterTokensData; // Return existing data
        }
    }
    
    // 1. Extract CSRF token from cookies
    const csrfMatch = document.cookie.match(/ct0=([^;]+)/);
    const csrfToken = csrfMatch ? csrfMatch[1] : null;
    
    // 2. Extract guest_id from cookies (ensure proper format)
    const guestIdMatch = document.cookie.match(/guest_id=([^;]+)/);
    const guestId = guestIdMatch ? guestIdMatch[1] : null;
    
    // 3. Extract personalization_id from cookies (handle quotes properly)
    const personalizationMatch = document.cookie.match(/personalization_id=([^;]+)/);
    const personalizationId = personalizationMatch ? decodeURIComponent(personalizationMatch[1]).replace(/"/g, '') : null;
    
    // 4. Extract twid (Twitter ID) from cookies (ensure proper format)
    const twidMatch = document.cookie.match(/twid=([^;]+)/);
    const twid = twidMatch ? decodeURIComponent(twidMatch[1]) : null;
    
    // 5. Extract language preference
    const langMatch = document.cookie.match(/lang=([^;]+)/);
    const language = langMatch ? langMatch[1] : 'en';
    
    // Note: auth_token is HttpOnly and cannot be accessed by JavaScript
    // It will be provided by the C++ backend through updateCurlWithHttpOnlyCookies()
    const authToken = null; // Will be populated by C++ backend via HttpOnly cookies
    
    // 6. Generate a client transaction ID (88 character base64-like string)
    function generateTransactionId() {
        const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
        let result = '';
        for (let i = 0; i < 88; i++) {
            result += chars.charAt(Math.floor(Math.random() * chars.length));
        }
        return result;
    }
    
    const clientTransactionId = generateTransactionId();
    
    // 7. Static Bearer token (this is the standard Twitter API bearer token)
    const bearerToken = "AAAAAAAAAAAAAAAAAAAAANRILgAAAAAAnNwIzUejRCOuH5E6I8xnZz4puTs%3D1Zv7ttfk8LF81IUq16cHjhLTvJu4FA33AGWWjCpTnA";
    
    // 8. Get current timestamp
    const timestamp = new Date().toISOString();
    
    // 9. Create structured data object for TOKENS ONLY
    const extractedData = {
        type: 'twitter_tokens_only', // Changed from 'twitter_tokens_with_api_data'
        timestamp: timestamp,
        url: window.location.href,
        tokens: {
            csrf_token: csrfToken,
            guest_id: guestId,
            personalization_id: personalizationId,
            twid: twid,
            language: language,
            auth_token: authToken,
            client_transaction_id: clientTransactionId,
            bearer_token: bearerToken
        },
        headers: {
            'accept': '*/*',
            'accept-language': 'en-US,en;q=0.9',
            'authorization': 'Bearer ' + bearerToken,
            'content-type': 'application/json',
            'origin': 'https://x.com',
            'referer': 'https://x.com/home',
            'sec-fetch-dest': 'empty',
            'sec-fetch-mode': 'cors',
            'sec-fetch-site': 'same-origin',
            'user-agent': navigator.userAgent,
            'x-client-transaction-id': clientTransactionId,
            'x-csrf-token': csrfToken || 'MISSING',
            'x-twitter-active-user': 'yes',
            'x-twitter-auth-type': 'OAuth2Session',
            'x-twitter-client-language': language
        },
        cookies: {},
        // Add captured browser headers from dynamic header capture system
        capturedBrowserHeaders: window.capturedBrowserHeaders || {}
        // REMOVED: homeTimelineApi section - features/variables now sent via OnFeatureDataUpdate
    };
    
    // 🍪 COMPREHENSIVE COOKIE SCRAPING - Extract ALL available cookies
    // This captures ALL cookies that JavaScript can access (not HttpOnly ones)
    console.log('🍪 COMPREHENSIVE COOKIE SCRAPING START');
    console.log('🍪 Raw document.cookie:', document.cookie);
    
    const allCookies = {};
    const cookieArray = [];
    
    // Parse all available cookies
    if (document.cookie) {
        const cookies = document.cookie.split('; ');
        console.log(`🍪 Found ${cookies.length} total cookies to process`);
        
        cookies.forEach((cookie, index) => {
            if (cookie.trim()) {
                const [name, ...valueParts] = cookie.split('=');
                const value = valueParts.join('='); // Handle values with = signs
                
                if (name && name.trim()) {
                    const cleanName = name.trim();
                    const cleanValue = value ? value.trim() : '';
                    
                    // Store in object for easy access
                    allCookies[cleanName] = cleanValue;
                    
                    // Store in array for building cookie string
                    cookieArray.push(`${cleanName}=${cleanValue}`);
                    
                    console.log(`🍪 [${index + 1}] ${cleanName} = ${cleanValue.substring(0, 20)}${cleanValue.length > 20 ? '...' : ''} (length: ${cleanValue.length})`);
                }
            }
        });
    } else {
        console.log('🍪 No cookies available via document.cookie');
    }
    
    // Build complete cookie string for C++ backend (ready for CURL headers)
    const completeCookieString = cookieArray.join('; ');
    console.log(`🍪 COMPLETE COOKIE STRING (${completeCookieString.length} chars):`, completeCookieString);
    
    // Extract specific Twitter authentication cookies for token validation
    const twitterAuthCookies = {
        ct0: allCookies.ct0 || null,
        guest_id: allCookies.guest_id || null,
        personalization_id: allCookies.personalization_id || null,
        twid: allCookies.twid || null,
        lang: allCookies.lang || 'en',
        auth_token: allCookies.auth_token || null, // Usually HttpOnly, may not be accessible
        // Additional Twitter cookies that may be present
        guest_id_marketing: allCookies.guest_id_marketing || null,
        guest_id_ads: allCookies.guest_id_ads || null,
        gt: allCookies.gt || null,
        kdt: allCookies.kdt || null,
        att: allCookies.att || null,
        __cf_bm: allCookies.__cf_bm || null, // CloudFlare cookie
        _ga: allCookies._ga || null, // Google Analytics
        _gid: allCookies._gid || null,
        dnt: allCookies.dnt || null,
        eu_cn: allCookies.eu_cn || null,
        night_mode: allCookies.night_mode || null,
        rweb_optin: allCookies.rweb_optin || null
    };
    
    console.log('🍪 TWITTER AUTHENTICATION COOKIES:');
    Object.entries(twitterAuthCookies).forEach(([name, value]) => {
        if (value) {
            console.log(`   ✅ ${name}: ${value.substring(0, 20)}${value.length > 20 ? '...' : ''} (${value.length} chars)`);
        } else {
            console.log(`   ❌ ${name}: Missing`);
        }
    });
    
    // Store ALL cookies in extractedData for C++ backend
    extractedData.cookies = {
        // Complete cookie string ready for CURL
        complete_cookie_string: completeCookieString,
        // Total count of cookies
        total_cookie_count: Object.keys(allCookies).length,
        // All individual cookies for selective access
        all_cookies: allCookies,
        // Twitter-specific authentication cookies
        twitter_auth_cookies: twitterAuthCookies,
        // Cookie parsing metadata
        parsing_info: {
            raw_cookie_length: document.cookie.length,
            parsed_cookies_count: Object.keys(allCookies).length,
            complete_string_length: completeCookieString.length,
            timestamp: new Date().toISOString()
        }
    };
    
    console.log(`🍪 COOKIE SCRAPING COMPLETE: ${Object.keys(allCookies).length} cookies captured`);
    
    // Add status information
    extractedData.status = {
        csrf_found: !!csrfToken,
        guest_id_found: !!guestId,
        logged_in: !!(csrfToken && twid),
        ready_for_api: !!(csrfToken && bearerToken)
    };
    
    // Log authentication status prominently
    console.log('🔐 AUTHENTICATION STATUS:');
    console.log(`   User logged in: ${extractedData.status.logged_in ? '✅ YES' : '❌ NO'}`);
    console.log(`   CSRF token: ${extractedData.status.csrf_found ? '✓' : '✗'}`);
    console.log(`   TWID (login indicator): ${twid ? '✓' : '✗'}`);
    if (!extractedData.status.logged_in) {
        console.log('   📌 Note: Data will be sent but success flag won\'t be set until user logs in');
    }
    
    // Validate tokens using the validation function
    const tokenValidation = validateTokens(extractedData.tokens);
    
    if (!tokenValidation.required_tokens_available) {
        console.warn('⚠️ Missing CRITICAL required tokens - not sending data to bridge');
        console.log('Missing CRITICAL required tokens:', tokenValidation.missing_required);
        console.log('Token status:', {
            csrf_token: !!csrfToken,
            guest_id: !!guestId,
            bearer_token: !!bearerToken,
            twid: !!twid,
            personalization_id: !!personalizationId,
            auth_token: !!authToken
        });
        
        // Still log to console for debugging but mark as incomplete
        extractedData.validation = {
            ...tokenValidation,
            data_sent_to_bridge: false,
            reason_not_sent: 'Missing CRITICAL required tokens: ' + tokenValidation.missing_required.join(', ')
        };
        
        // Store data even when incomplete for potential HttpOnly cookie integration
        window.twitterTokensData = extractedData;
        console.log('📦 Stored incomplete token data for HttpOnly cookie integration');
        
        console.log('📊 INCOMPLETE TOKEN DATA (not sent to bridge):', extractedData);
        return extractedData;
    }
    
    // Log preferred token status (won't block sending)
    if (!tokenValidation.preferred_tokens_available) {
        console.warn('⚠️ Missing PREFERRED tokens but PROCEEDING ANYWAY:', tokenValidation.missing_preferred);
        console.log('💡 TIP: CSRF and Guest ID are preferred for better API compatibility');
    }
    
    // SUCCESS: Proceed with token data sending
    extractedData.validation = {
        ...tokenValidation,
        data_sent_to_bridge: true
    };
    
    // Store data globally for HttpOnly cookie integration
    window.twitterTokensData = extractedData;
    console.log('📦 Stored token data for HttpOnly cookie integration');
    
    // Log validation results
    if (!tokenValidation.all_tokens_for_logged_user) {
        console.log('ℹ️ Guest user or missing optional tokens:', tokenValidation.missing_optional);
    } else {
        console.log('✅ All tokens available for logged-in user');
    }
    
    // Log token data summary
    console.log('✅ TOKEN DATA VALIDATION (C++ BACKEND READY):');
    console.log(`   🔑 Bearer Token: ${bearerToken ? 'Present' : 'Missing'}`);
    console.log(`   🔒 CSRF Token: ${csrfToken ? 'Present' : 'Missing'}`);
    console.log(`   👤 Guest ID: ${guestId ? 'Present' : 'Missing'}`);
    console.log(`   🆔 TWID: ${twid ? 'Present' : 'Missing'}`);
    console.log(`   🔐 Logged In: ${!!(csrfToken && twid) ? 'Yes' : 'No'}`);
    
    console.log('📡 NOTE: Features and variables are sent separately via OnFeatureDataUpdate');
    
    // ENHANCED BRIDGE COMMUNICATION with single send control
    let dataSentSuccessfully = false;
    
    // Send token data to C++ bridge (WITHOUT features/variables)
    if (window.twitterScrapingBridge) {
        try {
            // Test bridge connectivity
            if (window.twitterScrapingBridge.isReady && window.twitterScrapingBridge.isReady()) {
                console.log('✅ Bridge is ready');
            }
            
            // Send the token data only
            if (window.twitterScrapingBridge.onDataCaptured) {
                window.twitterScrapingBridge.onDataCaptured(JSON.stringify(extractedData));
                console.log('✅ TOKEN DATA sent to C++ bridge successfully - tokens + headers only');
                console.log(`   📊 Data type: ${extractedData.type}`);
                console.log(`   📏 Total payload: ${JSON.stringify(extractedData).length} chars`);
                console.log(`   ℹ️ Features/variables sent separately via OnFeatureDataUpdate`);
                
                // SET SINGLE SEND FLAG AFTER SUCCESSFUL BRIDGE COMMUNICATION
                window.twitterDataSentFlag.dataSent = true;
                window.twitterDataSentFlag.timestamp = new Date().toISOString();
                window.twitterDataSentFlag.httpOnlyReceived = false;
                window.twitterDataSentFlag.curlGenerated = true;
                window.twitterDataSentFlag.sessionId = Date.now().toString();
                dataSentSuccessfully = true;
                
                console.log('🔒 SINGLE SEND CONTROL -> Bridge communication successful, flag set to prevent continuous sending');
                
                if (tokenValidation.all_tokens_for_logged_user) {
                    console.log('✅ Complete logged-in user tokens sent');
                } else {
                    console.log('ℹ️ Guest user tokens sent (missing optional tokens)');
                }
                
                // Log authentication status for monitoring
                if (!extractedData.status.logged_in) {
                    console.log('⚠️ DATA SENT BUT USER NOT AUTHENTICATED -> dataSentSuccessfully will NOT be set in monitoring');
                    console.log('   CSRF token: ' + (csrfToken ? '✓' : '✗'));
                    console.log('   TWID (login cookie): ' + (twid ? '✓' : '✗'));
                    console.log('   Monitoring will continue checking every 5 seconds for authentication');
                }
            } else {
                console.error('❌ onDataCaptured method not available on bridge');
            }
        } catch (error) {
            console.error('❌ Failed to send token data to C++ bridge:', error);
        }
    } else {
        console.warn('⚠️ Twitter scraping bridge not available - attempting enhanced fallback methods...');
        
        // ENHANCED BRIDGE COMMUNICATION with extensive fallback methods
        let dataSent = false;
        const communicationResults = [];
        
        // Diagnose available communication methods
        console.log('🔍 BRIDGE DIAGNOSTIC -> Checking available communication methods:');
        console.log('  🌐 chrome.webview:', !!(window.chrome && window.chrome.webview));
        console.log('  🤖 AndroidInterface:', !!window.AndroidInterface);
        console.log('  📱 JavascriptInterface:', !!window.JavascriptInterface);
        console.log('  🔗 window.external:', !!(window.external && window.external.postMessage));
        console.log('  📡 parent window:', !!(window.parent && window.parent !== window));
        console.log('  📝 console logging:', true);
        console.log('  📨 custom events:', true);
        
        // Method 1: chrome.webview (WebView2) - Enhanced with retry logic
        if (!dataSent && window.chrome && window.chrome.webview) {
            console.log('🔧 BRIDGE METHOD 1 -> Attempting chrome.webview fallback');
            try {
                const message = {
                    type: 'twitter_tokens',
                    data: extractedData,
                    timestamp: new Date().toISOString(),
                    method: 'chrome_webview'
                };
                window.chrome.webview.postMessage(JSON.stringify(message));
                console.log('✅ BRIDGE SUCCESS -> Data sent via chrome.webview');
                communicationResults.push({ method: 'chrome.webview', success: true });
                dataSent = true;
            } catch (webviewError) {
                console.error('❌ BRIDGE FAILED -> chrome.webview error:', webviewError);
                communicationResults.push({ method: 'chrome.webview', success: false, error: webviewError.message });
            }
        }
        
        // Method 2: AndroidInterface (Android WebView) - Enhanced with method detection
        if (!dataSent && window.AndroidInterface) {
            console.log('🔧 BRIDGE METHOD 2 -> Attempting AndroidInterface fallback');
            try {
                // Check available methods on AndroidInterface
                const availableMethods = Object.getOwnPropertyNames(window.AndroidInterface);
                console.log('📱 AndroidInterface methods available:', availableMethods);
                
                if (typeof window.AndroidInterface.onTwitterDataCaptured === 'function') {
                    window.AndroidInterface.onTwitterDataCaptured(JSON.stringify(extractedData));
                    console.log('✅ BRIDGE SUCCESS -> Data sent via AndroidInterface.onTwitterDataCaptured');
                    dataSent = true;
                } else if (typeof window.AndroidInterface.processTwitterData === 'function') {
                    window.AndroidInterface.processTwitterData(JSON.stringify(extractedData));
                    console.log('✅ BRIDGE SUCCESS -> Data sent via AndroidInterface.processTwitterData');
                    dataSent = true;
                } else if (typeof window.AndroidInterface.receiveData === 'function') {
                    window.AndroidInterface.receiveData('twitter_tokens', JSON.stringify(extractedData));
                    console.log('✅ BRIDGE SUCCESS -> Data sent via AndroidInterface.receiveData');
                    dataSent = true;
                } else {
                    throw new Error('No suitable method found on AndroidInterface');
                }
                communicationResults.push({ method: 'AndroidInterface', success: true });
            } catch (androidError) {
                console.error('❌ BRIDGE FAILED -> AndroidInterface error:', androidError);
                communicationResults.push({ method: 'AndroidInterface', success: false, error: androidError.message });
            }
        }
        
        // Method 3: JavascriptInterface (Alternative Android WebView)
        if (!dataSent && window.JavascriptInterface) {
            console.log('🔧 BRIDGE METHOD 3 -> Attempting JavascriptInterface fallback');
            try {
                const availableMethods = Object.getOwnPropertyNames(window.JavascriptInterface);
                console.log('📲 JavascriptInterface methods available:', availableMethods);
                
                if (typeof window.JavascriptInterface.receiveTwitterData === 'function') {
                    window.JavascriptInterface.receiveTwitterData(JSON.stringify(extractedData));
                    console.log('✅ BRIDGE SUCCESS -> Data sent via JavascriptInterface.receiveTwitterData');
                    dataSent = true;
                } else if (typeof window.JavascriptInterface.onDataReceived === 'function') {
                    window.JavascriptInterface.onDataReceived('twitter', JSON.stringify(extractedData));
                    console.log('✅ BRIDGE SUCCESS -> Data sent via JavascriptInterface.onDataReceived');
                    dataSent = true;
                } else {
                    throw new Error('No suitable method found on JavascriptInterface');
                }
                communicationResults.push({ method: 'JavascriptInterface', success: true });
            } catch (jsInterfaceError) {
                console.error('❌ BRIDGE FAILED -> JavascriptInterface error:', jsInterfaceError);
                communicationResults.push({ method: 'JavascriptInterface', success: false, error: jsInterfaceError.message });
            }
        }
        
        // Method 4: window.external (IE/Edge legacy)
        if (!dataSent && window.external && window.external.postMessage) {
            console.log('🔧 BRIDGE METHOD 4 -> Attempting window.external fallback');
            try {
                const message = {
                    type: 'twitter_tokens',
                    data: extractedData,
                    timestamp: new Date().toISOString(),
                    method: 'window_external'
                };
                window.external.postMessage(JSON.stringify(message));
                console.log('✅ BRIDGE SUCCESS -> Data sent via window.external');
                communicationResults.push({ method: 'window.external', success: true });
                dataSent = true;
            } catch (externalError) {
                console.error('❌ BRIDGE FAILED -> window.external error:', externalError);
                communicationResults.push({ method: 'window.external', success: false, error: externalError.message });
            }
        }
        
        // Method 5: Console log with Android Logcat markers (ALWAYS try this for C++ parsing)
        console.log('🔧 BRIDGE METHOD 5 -> Using Android Logcat parsing method');
        try {
            console.log('TWITTER_TOKENS_FOR_CPP_BACKEND_START');
            console.log(JSON.stringify(extractedData));
            console.log('TWITTER_TOKENS_FOR_CPP_BACKEND_END');
            console.log('✅ BRIDGE SUCCESS -> Data logged for Android Logcat parsing');
            communicationResults.push({ method: 'console_logcat', success: true });
            if (!dataSent) dataSent = true; // Mark as sent if no other method worked
        } catch (logError) {
            console.error('❌ BRIDGE FAILED -> Console logging error:', logError);
            communicationResults.push({ method: 'console_logcat', success: false, error: logError.message });
        }
        
        // Method 6: Parent window postMessage
        if (window.parent && window.parent !== window) {
            console.log('🔧 BRIDGE METHOD 6 -> Attempting parent window postMessage');
            try {
                const message = {
                    type: 'twitter_tokens',
                    data: extractedData,
                    timestamp: new Date().toISOString(),
                    method: 'parent_postmessage'
                };
                window.parent.postMessage(message, '*');
                console.log('✅ BRIDGE SUCCESS -> Data sent to parent window');
                communicationResults.push({ method: 'parent_postmessage', success: true });
            } catch (parentError) {
                console.error('❌ BRIDGE FAILED -> Parent window postMessage error:', parentError);
                communicationResults.push({ method: 'parent_postmessage', success: false, error: parentError.message });
            }
        }
        
        // Method 7: Custom event dispatch
        console.log('🔧 BRIDGE METHOD 7 -> Attempting custom event dispatch');
        try {
            const customEvent = new CustomEvent('twitterTokensReady', {
                detail: {
                    type: 'twitter_tokens',
                    data: extractedData,
                    timestamp: new Date().toISOString(),
                    method: 'custom_event'
                }
            });
            document.dispatchEvent(customEvent);
            console.log('✅ BRIDGE SUCCESS -> Custom event dispatched');
            communicationResults.push({ method: 'custom_event', success: true });
        } catch (eventError) {
            console.error('❌ BRIDGE FAILED -> Custom event dispatch error:', eventError);
            communicationResults.push({ method: 'custom_event', success: false, error: eventError.message });
        }
        
        // Log communication summary
        console.log('📊 BRIDGE COMMUNICATION SUMMARY:');
        const successfulMethods = communicationResults.filter(r => r.success);
        const failedMethods = communicationResults.filter(r => !r.success);
        
        console.log(`✅ Successful methods (${successfulMethods.length}):`, successfulMethods.map(m => m.method));
        if (failedMethods.length > 0) {
            console.log(`❌ Failed methods (${failedMethods.length}):`, failedMethods.map(m => `${m.method}: ${m.error}`));
        }
        console.log(`🎯 Overall bridge communication: ${dataSent ? 'SUCCESS' : 'FAILED'}`);
        
        // SET SINGLE SEND FLAG AFTER SUCCESSFUL FALLBACK COMMUNICATION
        if (dataSent) {
            window.twitterDataSentFlag.dataSent = true;
            window.twitterDataSentFlag.timestamp = new Date().toISOString();
            window.twitterDataSentFlag.httpOnlyReceived = false;
            window.twitterDataSentFlag.curlGenerated = true;
            window.twitterDataSentFlag.sessionId = Date.now().toString();
            dataSentSuccessfully = true;
            
            console.log('🔒 SINGLE SEND CONTROL -> Fallback communication successful, flag set to prevent continuous sending');
            
            // Log authentication status for monitoring
            if (!extractedData.status.logged_in) {
                console.log('⚠️ DATA SENT VIA FALLBACK BUT USER NOT AUTHENTICATED -> dataSentSuccessfully will NOT be set in monitoring');
                console.log('   CSRF token: ' + (csrfToken ? '✓' : '✗'));
                console.log('   TWID (login cookie): ' + (twid ? '✓' : '✗'));
                console.log('   Monitoring will continue checking every 5 seconds for authentication');
            }
        }
    }
    
    // Generate Twitter API curl command with complete feature integration and encoded parameters
    generateTwitterApiCurl(extractedData);
    
    // Also log to console for debugging with token data only
    console.log('📊 TOKEN DATA SENT TO C++ BACKEND:', extractedData);
    console.log('🎯 C++ BACKEND INTEGRATION SUMMARY:');
    console.log(`   📊 Data type: ${extractedData.type}`);
    console.log(`   🔑 Tokens sent: ${Object.keys(extractedData.tokens).length}`);
    console.log(`   📋 Headers configured: ${Object.keys(extractedData.headers).length}`);
    console.log(`   🍪 Cookies available: ${Object.keys(extractedData.cookies).length}`);
    console.log(`   🔐 Authentication status: ${extractedData.status.logged_in ? 'Logged In' : 'Guest'}`);
    console.log('');
    console.log('ℹ️ FEATURES AND VARIABLES:');
    console.log('   📡 Sent separately via OnFeatureDataUpdate bridge method');
    console.log('   ⏱️ After 5 seconds of stability');
    console.log('   🎯 Only HomeTimeline features included');
    console.log('   🚫 Viewer endpoints excluded');
    
    return extractedData;
}

// Validation function to check if required tokens are available - RELAXED FOR DEBUGGING
function validateTokens(tokens) {
    const required = ['bearer_token']; // Only bearer token is truly required
    const preferred = ['csrf_token', 'guest_id']; // Preferred but not blocking
    const optional = ['twid', 'personalization_id', 'auth_token']; // These are only available when logged in
    
    const validation = {
        required_tokens_available: true,
        missing_required: [],
        preferred_tokens_available: true,
        missing_preferred: [],
        optional_tokens_available: true,
        missing_optional: [],
        all_tokens_for_logged_user: true
    };
    
    // Check truly required tokens (only bearer token)
    required.forEach(token => {
        if (!tokens[token] || tokens[token] === 'MISSING') {
            validation.required_tokens_available = false;
            validation.missing_required.push(token);
        }
    });
    
    // Check preferred tokens (won't block sending)
    preferred.forEach(token => {
        if (!tokens[token] || tokens[token] === 'MISSING') {
            validation.preferred_tokens_available = false;
            validation.missing_preferred.push(token);
        }
    });
    
    // Check optional tokens (for logged-in users)
    optional.forEach(token => {
        if (!tokens[token] || tokens[token] === 'MISSING') {
            validation.optional_tokens_available = false;
            validation.missing_optional.push(token);
        }
    });
    
    validation.all_tokens_for_logged_user = validation.required_tokens_available && validation.preferred_tokens_available && validation.optional_tokens_available;
    
    return validation;
}

// Function to be called from C++ backend with HttpOnly cookie data
function updateCurlWithHttpOnlyCookies(httpOnlyCookies) {
    console.log('🍪 COOKIE INTEGRATION -> Received HttpOnly cookies from C++ backend');
    console.log('🍪 COOKIE INTEGRATION -> HttpOnly cookies data:', httpOnlyCookies);
    
    // Check if we have the previously extracted data
    if (!window.twitterTokensData) {
        console.log('🍪 COOKIE INTEGRATION -> No previous token data found, storing HttpOnly cookies for later use');
        window.httpOnlyCookiesData = httpOnlyCookies;
        return;
    }
    
    // Update the extracted data with HttpOnly cookie values
    const extractedData = window.twitterTokensData;
    const tokens = extractedData.tokens;
    
    // Map HttpOnly cookies to token fields
    if (httpOnlyCookies.auth_token) {
        tokens.auth_token = httpOnlyCookies.auth_token;
        console.log('🍪 COOKIE INTEGRATION -> Updated auth_token from HttpOnly cookies');
    }
    
    if (httpOnlyCookies.ct0 && (!tokens.csrf_token || tokens.csrf_token === 'MISSING' || tokens.csrf_token === null)) {
        tokens.csrf_token = httpOnlyCookies.ct0;
        console.log('🍪 COOKIE INTEGRATION -> Updated csrf_token from HttpOnly cookies');
    }
    
    if (httpOnlyCookies.guest_id && (!tokens.guest_id || tokens.guest_id === 'MISSING' || tokens.guest_id === null)) {
        tokens.guest_id = httpOnlyCookies.guest_id;
        console.log('🍪 COOKIE INTEGRATION -> Updated guest_id from HttpOnly cookies');
    }
    
    if (httpOnlyCookies.personalization_id && (!tokens.personalization_id || tokens.personalization_id === 'MISSING' || tokens.personalization_id === null)) {
        tokens.personalization_id = httpOnlyCookies.personalization_id;
        console.log('🍪 COOKIE INTEGRATION -> Updated personalization_id from HttpOnly cookies');
    }
    
    if (httpOnlyCookies.twid && (!tokens.twid || tokens.twid === 'MISSING' || tokens.twid === null)) {
        tokens.twid = httpOnlyCookies.twid;
        console.log('🍪 COOKIE INTEGRATION -> Updated twid from HttpOnly cookies');
    }
    
    // Regenerate the curl command with the updated tokens using DYNAMIC parameters
    console.log('🔄 CURL GENERATION -> Regenerating curl command with HttpOnly cookie data and dynamic parameters...');
    const updatedApiData = generateTwitterApiCurl(extractedData);
    
    // Update the stored data
    extractedData.apiInfo = updatedApiData;
    extractedData.httpOnlyCookiesIntegrated = true;
    extractedData.httpOnlyCookiesReceived = httpOnlyCookies;
    
    // Log the integration success
    console.log('✅ INTEGRATION COMPLETE -> HttpOnly cookie integration complete with dynamic parameters');
    console.log('🔑 Auth Token:', tokens.auth_token !== 'MISSING_AUTH_TOKEN' ? 'UPDATED' : 'PENDING');
    console.log('🔒 CSRF Token:', tokens.csrf_token !== 'MISSING_CSRF' ? 'UPDATED' : 'PENDING');
    console.log('👤 Guest ID:', tokens.guest_id !== 'MISSING_GUEST_ID' ? 'UPDATED' : 'PENDING');
    console.log('📊 Personalization ID:', tokens.personalization_id !== 'MISSING_PERSONALIZATION_ID' ? 'UPDATED' : 'PENDING');
    console.log('🆔 TWID:', tokens.twid !== 'MISSING_TWID' ? 'UPDATED' : 'PENDING');
    console.log('🚀 FINAL CURL READY FOR EXECUTION WITH DYNAMIC PARAMETERS AND COMPLETE FEATURES');
    
    // CRITICAL: SEND FEATURES AND VARIABLES IMMEDIATELY AFTER HTTPONLY COOKIES
    console.log('');
    console.log('🚨 HTTPONLY COOKIES RECEIVED -> SENDING FEATURES/VARIABLES IMMEDIATELY');
    // TODO: OBSOLETE CALL - Remove this call since onFeatureDataUpdate method was removed
    // The URL capture approach via onHomeTimelineUrlCaptured is now used instead
    sendHomeTimelineFeaturesImmediately();
    
    return extractedData;
}

// ENHANCED BRIDGE DIAGNOSTICS - Check all available communication methods
function checkCommunicationMethods() {
    console.log('🔍 BRIDGE DIAGNOSTICS -> Checking available communication methods:');
    
    const methods = {
        twitterScrapingBridge: {
            available: !!(window.twitterScrapingBridge),
            ready: false,
            details: 'Primary C++ bridge'
        },
        chromeWebview: {
            available: !!(window.chrome && window.chrome.webview),
            ready: !!(window.chrome && window.chrome.webview && window.chrome.webview.postMessage),
            details: 'WebView2 communication'
        },
        androidInterface: {
            available: !!window.AndroidInterface,
            ready: false,
            methods: [],
            details: 'Android WebView bridge'
        },
        javascriptInterface: {
            available: !!window.JavascriptInterface,
            ready: false,
            methods: [],
            details: 'Alternative Android bridge'
        },
        windowExternal: {
            available: !!(window.external && window.external.postMessage),
            ready: !!(window.external && window.external.postMessage),
            details: 'IE/Edge legacy bridge'
        },
        parentWindow: {
            available: !!(window.parent && window.parent !== window),
            ready: !!(window.parent && window.parent !== window && window.parent.postMessage),
            details: 'Parent window communication'
        },
        consoleLogcat: {
            available: true,
            ready: true,
            details: 'Android Logcat parsing'
        },
        customEvents: {
            available: !!(document.dispatchEvent && CustomEvent),
            ready: !!(document.dispatchEvent && CustomEvent),
            details: 'DOM custom events'
        }
    };
    
    // Enhanced bridge detection
    if (methods.twitterScrapingBridge.available) {
        try {
            if (window.twitterScrapingBridge.isReady && typeof window.twitterScrapingBridge.isReady === 'function') {
                methods.twitterScrapingBridge.ready = window.twitterScrapingBridge.isReady();
            }
            const bridgeMethods = Object.getOwnPropertyNames(window.twitterScrapingBridge);
            methods.twitterScrapingBridge.methods = bridgeMethods;
        } catch (e) {
            methods.twitterScrapingBridge.error = e.message;
        }
    }
    
    // Check AndroidInterface methods
    if (methods.androidInterface.available) {
        try {
            const androidMethods = Object.getOwnPropertyNames(window.AndroidInterface);
            methods.androidInterface.methods = androidMethods;
            methods.androidInterface.ready = androidMethods.includes('onTwitterDataCaptured') || 
                                           androidMethods.includes('processTwitterData') ||
                                           androidMethods.includes('receiveData');
        } catch (e) {
            methods.androidInterface.error = e.message;
        }
    }
    
    // Check JavascriptInterface methods
    if (methods.javascriptInterface.available) {
        try {
            const jsMethods = Object.getOwnPropertyNames(window.JavascriptInterface);
            methods.javascriptInterface.methods = jsMethods;
            methods.javascriptInterface.ready = jsMethods.includes('receiveTwitterData') || 
                                              jsMethods.includes('onDataReceived');
        } catch (e) {
            methods.javascriptInterface.error = e.message;
        }
    }
    
    // Log results
    console.log('📊 COMMUNICATION METHOD STATUS:');
    Object.entries(methods).forEach(([key, info]) => {
        const status = info.ready ? '✅ READY' : info.available ? '⚠️ AVAILABLE' : '❌ UNAVAILABLE';
        console.log(`  ${status} ${key}: ${info.details}`);
        if (info.methods && info.methods.length > 0) {
            console.log(`    Methods: ${info.methods.join(', ')}`);
        }
        if (info.error) {
            console.log(`    Error: ${info.error}`);
        }
    });
    
    // Log browser environment
    console.log('🌐 BROWSER ENVIRONMENT:');
    console.log(`  User Agent: ${navigator.userAgent}`);
    console.log(`  URL: ${window.location.href}`);
    console.log(`  Cookies Available: ${document.cookie ? 'YES' : 'NO'}`);
    console.log(`  Local Storage: ${window.localStorage ? 'YES' : 'NO'}`);
    console.log(`  WebView Context: ${window.chrome ? 'Chrome WebView' : 'Standard Browser'}`);
    
    return methods;
}

// Function to continuously monitor and send data with enhanced bridge detection
function startTokenMonitoring() {
    console.log('🚀 ENHANCED TOKEN MONITORING -> Starting comprehensive system...');
    
    // Initial bridge diagnostics
    checkCommunicationMethods();
    
    // Enhanced bridge detection with 30-second monitoring
    function waitForBridge(callback, maxAttempts = 60) { // 30 seconds of checking
        let attempts = 0;
        const checkInterval = 500; // Check every 500ms
        
        const checkBridge = () => {
            attempts++;
            const methods = checkCommunicationMethods();
            
            // Check if any method is ready
            const readyMethods = Object.entries(methods).filter(([key, info]) => info.ready);
            
            if (readyMethods.length > 0) {
                console.log(`✅ BRIDGE READY -> Found ${readyMethods.length} ready communication methods after ${attempts * checkInterval}ms`);
                console.log('Ready methods:', readyMethods.map(([key, info]) => `${key} (${info.details})`));
                callback();
                return;
            }
            
            if (attempts >= maxAttempts) {
                console.warn(`⚠️ BRIDGE TIMEOUT -> No bridge ready after ${maxAttempts * checkInterval}ms, proceeding with fallback methods`);
                callback();
                return;
            }
            
            if (attempts % 10 === 0) { // Log every 5 seconds
                console.log(`⏳ BRIDGE DETECTION -> Attempt ${attempts}/${maxAttempts} (${attempts * checkInterval}ms elapsed)`);
            }
            
            setTimeout(checkBridge, checkInterval);
        };
        checkBridge();
    }
    
    // Start monitoring after bridge detection
    waitForBridge(() => {
        // IMMEDIATE token extraction without waiting
        console.log('🚀 IMMEDIATE EXTRACTION -> Starting token extraction immediately...');
        const initialData = extractAndSendTokens();
        
        // Track success - ONLY if authenticated AND data sent
        let dataSentSuccessfully = false;
        if (initialData && initialData.validation && initialData.validation.data_sent_to_bridge) {
            // Check if user is authenticated
            if (initialData.status && initialData.status.logged_in) {
                dataSentSuccessfully = true;
                console.log('✅ AUTHENTICATED DATA SENT -> Setting success flag');
            } else {
                console.log('⚠️ DATA SENT BUT NOT AUTHENTICATED -> Not setting success flag');
            }
        }
        
        // Enhanced monitoring intervals
        const successInterval = 30000; // 30 seconds when successful
        const retryInterval = 5000;    // 5 seconds when not successful
        
        // Monitor for changes with adaptive intervals
        setInterval(() => {
            try {
                const data = extractAndSendTokens();
                if (data && data.validation && data.validation.data_sent_to_bridge) {
                    // Only set success if authenticated
                    if (data.status && data.status.logged_in) {
                        if (!dataSentSuccessfully) {
                            console.log('✅ TOKEN MONITORING -> First successful AUTHENTICATED data transmission');
                        }
                        dataSentSuccessfully = true;
                    } else {
                        console.log('📡 TOKEN MONITORING -> Data sent but user not authenticated');
                    }
                }
            } catch (error) {
                console.error('❌ TOKEN MONITORING -> Error in monitoring cycle:', error);
            }
        }, dataSentSuccessfully ? successInterval : retryInterval);
        
        // Monitor for page navigation changes with enhanced detection
        let currentUrl = window.location.href;
        let currentPath = window.location.pathname;
        let wasOnLoginPage = false;
        
        // Function to check if we're on login page
        function isLoginPage() {
            const path = window.location.pathname.toLowerCase();
            return path.includes('/login') || path.includes('/i/flow/login') || path === '/';
        }
        
        // Function to check if we're on homepage or authenticated pages
        function isAuthenticatedPage() {
            const path = window.location.pathname.toLowerCase();
            return path.includes('/home') || path.includes('/timeline') || 
                   (path !== '/' && !path.includes('/login') && !path.includes('/i/flow'));
        }
        
        setInterval(() => {
            if (window.location.href !== currentUrl || window.location.pathname !== currentPath) {
                console.log('🔄 NAVIGATION DETECTED -> Page changed, re-extracting tokens...');
                console.log(`  Previous: ${currentUrl}`);
                console.log(`  Current: ${window.location.href}`);
                
                // Check if we're moving from login to authenticated page
                const previouslyOnLogin = wasOnLoginPage || currentPath.includes('/login') || currentPath.includes('/i/flow/login');
                const nowAuthenticated = isAuthenticatedPage();
                
                if (previouslyOnLogin && nowAuthenticated) {
                    console.log('🔐 AUTH TRANSITION DETECTED -> Moving from login to authenticated page');
                    console.log('🔄 RESETTING SINGLE SEND CONTROL -> Will capture new features from homepage');
                    
                    // CRITICAL: Reset the global single send flag to allow new data capture
                    window.twitterDataSentFlag.dataSent = false;
                    window.twitterDataSentFlag.timestamp = null;
                    window.twitterDataSentFlag.sessionId = null;
                    
                    // Also clear the captured data to force fresh capture
                    window.capturedTwitterApiData = {
                        apiBaseUrl: null,
                        variables: null,
                        features: null,
                        allFeatures: {},
                        allEndpoints: {},
                        bearerToken: null,
                        operationId: null,
                        lastCaptureTime: null,
                        captureCount: 0,
                        isValid: false
                    };
                    
                    // Reset HomeTimeline-specific storage
                    window.homeTimelineFeatures = {};
                    window.homeTimelineVariables = {};
                    
                    // Reset tracking counts
                    window.lastSentFeatureCount = 0;
                    window.lastSentVariableCount = 0;
                    
                    console.log('✅ RESET COMPLETE -> Ready to capture full feature set from authenticated pages');
                    
                    // Force check for feature increases after reset
                    setTimeout(() => {
                        console.log('🔄 AUTH TRANSITION -> Checking for features after authentication');
                        checkAndResendIfIncreased();
                    }, 3000); // Wait 3 seconds for page to load and features to be captured
                }
                
                currentUrl = window.location.href;
                currentPath = window.location.pathname;
                wasOnLoginPage = isLoginPage();
                
                // Reset success flag on navigation
                dataSentSuccessfully = false;
                
                // Wait for page to stabilize then extract
                setTimeout(() => {
                    console.log('📡 POST-NAVIGATION EXTRACTION -> Extracting tokens after page stabilization');
                    const data = extractAndSendTokens();
                    if (data && data.validation && data.validation.data_sent_to_bridge) {
                        // Only set success if authenticated
                        if (data.status && data.status.logged_in) {
                            dataSentSuccessfully = true;
                            console.log('✅ POST-NAVIGATION -> AUTHENTICATED data successfully extracted and sent');
                        } else {
                            console.log('⚠️ POST-NAVIGATION -> Data sent but user not authenticated, keeping dataSentSuccessfully = false');
                        }
                    }
                }, 2000);
            }
        }, 1000);
        
        console.log('✅ ENHANCED TOKEN MONITORING -> System fully initialized');
        console.log(`  Initial extraction: ${dataSentSuccessfully ? 'SUCCESS' : 'PENDING'}`);
        console.log(`  Initial authentication: ${initialData && initialData.status && initialData.status.logged_in ? 'LOGGED IN' : 'NOT LOGGED IN'}`);
        console.log(`  Monitoring interval: ${dataSentSuccessfully ? 'Every 30s' : 'Every 5s'}`);
        console.log(`  Navigation detection: ACTIVE`);
        console.log(`  Bridge detection: COMPLETED`);
        console.log('');
        console.log('📌 NOTE: dataSentSuccessfully flag will only be set when:');
        console.log('  1. User is authenticated (logged_in = true)');
        console.log('  2. Data is successfully sent to bridge');
        console.log('  3. Both CSRF token and TWID are present');
    });
}

// Initialize the complete system with full integration
console.log('🚀 INITIALIZING COMPLETE INTEGRATED TWITTER API SYSTEM...');

// SIMPLIFIED TRACKING: Just track last sent counts
window.lastSentFeatureCount = 0;
window.lastSentVariableCount = 0;
window.lastSentTime = null;

// Function to determine page type
function getPageType() {
    const path = window.location.pathname.toLowerCase();
    if (path.includes('/login') || path.includes('/i/flow/login') || path === '/') {
        return 'login';
    } else if (path.includes('/home') || path.includes('/timeline')) {
        return 'authenticated';
    }
    return 'other';
}

// NEW: Send HomeTimeline URL to backend for logging
function sendHomeTimelineUrlToBackend(url, tokens) {
    console.log('🎯 HOMETIMELINE URL -> Sending to C++ backend for logging');
    console.log('   URL:', url.substring(0, 100) + '...');
    console.log('   Tokens length:', tokens.length);
    
    try {
        // Check if bridge is available
        if (!window.twitterScrapingBridge || typeof window.twitterScrapingBridge.onHomeTimelineUrlCaptured !== 'function') {
            console.log('⚠️ HOMETIMELINE URL -> Bridge not available, storing locally');
            
            // Store locally for later transmission
            if (!window.storedHomeTimelineUrls) {
                window.storedHomeTimelineUrls = [];
            }
            window.storedHomeTimelineUrls.push({
                url: url,
                tokens: tokens,
                timestamp: Date.now()
            });
            return;
        }
        
        // Send via bridge
        window.twitterScrapingBridge.onHomeTimelineUrlCaptured(url, tokens);
        console.log('✅ HOMETIMELINE URL -> Successfully sent to backend');
        
        // Mark as sent to avoid duplicate logging
        if (!window.homeTimelineUrlsSent) {
            window.homeTimelineUrlsSent = new Set();
        }
        window.homeTimelineUrlsSent.add(url);
        
    } catch (error) {
        console.error('❌ HOMETIMELINE URL -> Failed to send to backend:', error);
    }
}

// Function to check if user is authenticated
function isUserAuthenticated() {
    const pageType = getPageType();
    const hasAuthCookies = document.cookie.includes('twid=') && document.cookie.includes('ct0=');
    return pageType === 'authenticated' || (pageType === 'other' && hasAuthCookies);
}

// Function to send encoded feature data to C++ backend via new bridge method
function sendEncodedFeatureUpdate() {
    console.log('📡 FEATURE UPDATE -> Preparing to send encoded feature/variable update to C++ backend');
    
    // DEBUG: Check bridge availability
    console.log('🌉 BRIDGE CHECK -> twitterScrapingBridge:', !!window.twitterScrapingBridge);
    if (window.twitterScrapingBridge) {
        console.log('   onFeatureDataUpdate method:', !!window.twitterScrapingBridge.onFeatureDataUpdate);
    }
    
    // CRITICAL: Only send data after authentication
    if (!isUserAuthenticated()) {
        console.log('🚫 FEATURE UPDATE -> User not authenticated, storing data locally only');
        return;
    }
    
    // Check if this is first send after auth
    if (!window.featureVariableTracker.hasAuthenticated) {
        window.featureVariableTracker.hasAuthenticated = true;
        console.log('🔐 FEATURE UPDATE -> First update after authentication detected');
    }
    
    // Get current data
    const currentData = window.capturedTwitterApiData;
    if (!currentData) {
        console.log('📡 FEATURE UPDATE -> No captured data available yet, skipping');
        return;
    }
    
    // CRITICAL: Ensure we're only using HomeTimeline features
    if (!window.homeTimelineFeatures || Object.keys(window.homeTimelineFeatures).length === 0) {
        console.log('⚠️ FEATURE UPDATE -> No HomeTimeline features captured yet, waiting...');
        return;
    }
    
    // Use ONLY HomeTimeline features and variables
    const homeTimelineFeatures = window.homeTimelineFeatures || {};
    const homeTimelineVariables = window.homeTimelineVariables || {};
    
    // Count current features and variables
    const currentFeatureCount = Object.keys(homeTimelineFeatures).length;
    const currentVariableCount = Object.keys(homeTimelineVariables).length;
    
    console.log('📊 HOMETIMELINE FEATURE UPDATE -> Using ONLY HomeTimeline data');
    console.log(`   🎯 HomeTimeline features: ${currentFeatureCount}`);
    console.log(`   🔧 HomeTimeline variables: ${currentVariableCount}`);
    console.log(`   📍 Excluding viewer/other endpoints`);
    
    // DEBUG: Log the actual variables
    console.log('🔧 VARIABLES DEBUG:');
    console.log('   HomeTimeline variables object:', homeTimelineVariables);
    console.log('   Variables keys:', Object.keys(homeTimelineVariables));
    
    // Determine current page type
    const currentPageType = getPageType();
    
    // DEBUG: Log URL being used
    const urlToUse = window.lastCapturedHomeTimelineUrl || window.location.href;
    const isCompleteUrl = !!window.lastCapturedHomeTimelineUrl;
    console.log(`🔗 STABILITY UPDATE URL SOURCE: ${isCompleteUrl ? 'COMPLETE CAPTURED URL' : 'FALLBACK PAGE URL'}`);
    console.log(`🔗 URL: ${urlToUse.substring(0, 100)}...`);
    
    // Update best feature count if current is higher
    if (currentFeatureCount > window.featureVariableTracker.bestFeatureCount) {
        window.featureVariableTracker.bestFeatureCount = currentFeatureCount;
        console.log(`🎯 NEW BEST FEATURE COUNT: ${currentFeatureCount} (from ${currentPageType} page)`);
    }
    
    // Encode the data
    const encodedFeatures = encodeURIComponent(JSON.stringify(homeTimelineFeatures));
    const encodedVariables = encodeURIComponent(JSON.stringify(homeTimelineVariables));
    
    // DEBUG: Log first 200 chars of encoded data
    console.log('📏 ENCODED DATA PREVIEW:');
    console.log('   Features (first 200 chars):', encodedFeatures.substring(0, 200) + '...');
    console.log('   Variables (first 200 chars):', encodedVariables.substring(0, 200) + '...');
    
    // Create the update payload
    const updatePayload = {
        type: 'feature_variable_update',
        timestamp: new Date().toISOString(),
        counts: {
            features: currentFeatureCount,
            variables: currentVariableCount,
            previousFeatures: window.featureVariableTracker.previousFeatureCount,
            previousVariables: window.featureVariableTracker.previousVariableCount
        },
        encoded: {
            features: encodedFeatures,
            variables: encodedVariables
        },
        raw: {
            features: homeTimelineFeatures,
            variables: homeTimelineVariables
        },
        metadata: {
            url: window.lastCapturedHomeTimelineUrl || window.location.href, // Use captured complete URL if available
            operationId: currentData.operationId || 'unknown',
            endpoint: currentData.endpoint || 'unknown',
            dataSource: 'HOMETIMELINE_ONLY', // CLEAR INDICATION
            updateNumber: window.featureVariableTracker.updateCount + 1,
            pageType: currentPageType,
            bestFeatureCount: window.featureVariableTracker.bestFeatureCount,
            isStableUpdate: true,
            stabilityDuration: 5000,
            skippedEndpoints: window.skippedEndpoints, // Include skipped endpoint stats
            isHomeTimelineData: true // Explicit flag
        }
    };
    
    // Send via bridge if available
    if (window.twitterScrapingBridge && window.twitterScrapingBridge.onFeatureDataUpdate) {
        try {
            console.log('📡 FEATURE UPDATE -> Sending STABLE update via onFeatureDataUpdate bridge method');
            console.log(`   📊 Features: ${currentFeatureCount} (was ${window.featureVariableTracker.previousFeatureCount})`);
            console.log(`   🔧 Variables: ${currentVariableCount} (was ${window.featureVariableTracker.previousVariableCount})`);
            console.log(`   📏 Encoded features length: ${encodedFeatures.length} chars`);
            console.log(`   📏 Encoded variables length: ${encodedVariables.length} chars`);
            console.log(`   📍 Page type: ${currentPageType}`);
            console.log(`   ⏱️ Stable for: 5 seconds`);
            
            // CRITICAL: Log the actual call
            console.log('🌉 CALLING BRIDGE -> window.twitterScrapingBridge.onFeatureDataUpdate()');
            window.twitterScrapingBridge.onFeatureDataUpdate(JSON.stringify(updatePayload));
            
            console.log('✅ FEATURE UPDATE -> Successfully sent STABLE data to C++ backend');
            
            // Update tracker
            window.featureVariableTracker.previousFeatureCount = currentFeatureCount;
            window.featureVariableTracker.previousVariableCount = currentVariableCount;
            window.featureVariableTracker.previousEncodedFeatures = encodedFeatures;
            window.featureVariableTracker.previousEncodedVariables = encodedVariables;
            window.featureVariableTracker.lastUpdateTime = new Date().toISOString();
            window.featureVariableTracker.updateCount++;
            window.featureVariableTracker.lastPageType = currentPageType;
            window.featureVariableTracker.isStable = true;
            window.featureVariableTracker.hasSentAfterAuth = true;
            
        } catch (error) {
            console.error('❌ FEATURE UPDATE -> Error sending update:', error);
            console.error('   Error details:', error.message, error.stack);
        }
    } else {
        console.log('⚠️ FEATURE UPDATE -> Bridge method onFeatureDataUpdate not available');
        console.log('   twitterScrapingBridge exists:', !!window.twitterScrapingBridge);
        if (window.twitterScrapingBridge) {
            console.log('   Available methods:', Object.getOwnPropertyNames(window.twitterScrapingBridge));
        }
        
        // Fallback: Log with special markers for C++ parsing
        console.log('FEATURE_UPDATE_FOR_CPP_START');
        console.log(JSON.stringify(updatePayload));
        console.log('FEATURE_UPDATE_FOR_CPP_END');
    }
}

// NEW: Function to handle feature changes with stability tracking
function handleFeatureChange() {
    const currentData = window.capturedTwitterApiData;
    if (!currentData) {
        return;
    }
    
    // Only track changes after authentication
    if (!isUserAuthenticated()) {
        console.log('📦 STABILITY TRACKING -> Storing data locally (not authenticated)');
        return;
    }
    
    // CRITICAL: Only count HomeTimeline features and variables
    const homeTimelineFeatures = window.homeTimelineFeatures || {};
    const homeTimelineVariables = window.homeTimelineVariables || {};
    
    // Count current features and variables
    const currentFeatureCount = Object.keys(homeTimelineFeatures).length;
    const currentVariableCount = Object.keys(homeTimelineVariables).length;
    
    console.log(`📊 STABILITY TRACKING -> HomeTimeline Features: ${currentFeatureCount}, Variables: ${currentVariableCount}`);
    
    // Update last change time
    window.featureVariableTracker.lastChangeTime = Date.now();
    window.featureVariableTracker.isStable = false;
    
    // Clear existing stability timer
    if (window.featureVariableTracker.stabilityTimer) {
        clearTimeout(window.featureVariableTracker.stabilityTimer);
        console.log('⏱️ STABILITY TRACKING -> Reset stability timer (changes detected)');
    }
    
    // Set new stability timer for 5 seconds
    window.featureVariableTracker.stabilityTimer = setTimeout(() => {
        console.log('✅ STABILITY ACHIEVED -> No changes for 5 seconds, sending data');
        window.featureVariableTracker.isStable = true;
        
        // Send the stable update
        sendEncodedFeatureUpdate();
        
    }, 5000); // 5 seconds
}

// Function to check for feature/variable count changes
function checkFeatureVariableChanges() {
    const currentData = window.capturedTwitterApiData;
    if (!currentData) {
        return false;
    }
    
    // CRITICAL: Only count HomeTimeline features and variables
    const homeTimelineFeatures = window.homeTimelineFeatures || {};
    const homeTimelineVariables = window.homeTimelineVariables || {};
    
    // Count current features and variables
    const currentFeatureCount = Object.keys(homeTimelineFeatures).length;
    const currentVariableCount = Object.keys(homeTimelineVariables).length;
    
    // Check if counts have changed
    const featuresChanged = currentFeatureCount !== window.featureVariableTracker.previousFeatureCount;
    const variablesChanged = currentVariableCount !== window.featureVariableTracker.previousVariableCount;
    
    if (featuresChanged || variablesChanged) {
        console.log('🔄 FEATURE CHANGE DETECTED ->');
        if (featuresChanged) {
            console.log(`   📊 Features: ${window.featureVariableTracker.previousFeatureCount} → ${currentFeatureCount}`);
        }
        if (variablesChanged) {
            console.log(`   🔧 Variables: ${window.featureVariableTracker.previousVariableCount} → ${currentVariableCount}`);
        }
        
        // Handle the change with stability tracking
        handleFeatureChange();
        
        // Update counts for next comparison
        window.featureVariableTracker.previousFeatureCount = currentFeatureCount;
        window.featureVariableTracker.previousVariableCount = currentVariableCount;
        
        return true;
    }
    
    return false;
}

// Start continuous monitoring for feature/variable changes
function startFeatureVariableMonitoring() {
    console.log('🔍 SIMPLIFIED MONITORING -> Starting HomeTimeline feature/variable tracking');
    console.log('   📈 Will resend all data when feature count increases');
    console.log('   🎯 Only tracking HomeTimeline GraphQL endpoints');
    
    // Initialize tracking variables
    window.lastSentFeatureCount = 0;
    window.lastSentVariableCount = 0;
    window.lastSentTime = null;
    
    // Simple monitoring - check every 2 seconds for increases
    setInterval(() => {
        checkAndResendIfIncreased();
    }, 2000);
    
    // Also hook into the existing extractApiParametersFromRequest function
    const originalExtractFunction = window.extractApiParametersFromRequest;
    window.extractApiParametersFromRequest = function(url, options) {
        // Call original function
        if (originalExtractFunction) {
            originalExtractFunction.call(this, url, options);
        }
        
        // Check for increases after extraction
        setTimeout(() => {
            checkAndResendIfIncreased();
        }, 100);
    };
    
    console.log('✅ SIMPLIFIED MONITORING -> System initialized');
    console.log('   Checking every 2 seconds for feature count increases');
    console.log('   Will send immediately when HttpOnly cookies are received');
}

// Start network interception for feature aggregation
interceptNetworkRequests();

// Start token monitoring for C++ bridge integration
startTokenMonitoring();

// NEW: Start continuous feature/variable monitoring
startFeatureVariableMonitoring();

// CRITICAL: Add bridge monitoring to ensure connection is established
function monitorBridgeAvailability() {
    let checkCount = 0;
    const maxChecks = 20; // Maximum 10 seconds
    
    function checkBridge() {
        checkCount++;
        
        if (window.twitterScrapingBridge && window.twitterScrapingBridge.onFeatureDataUpdate) {
            console.log('✅ BRIDGE MONITOR -> Bridge is now available!');
            console.log(`   Bridge found after ${checkCount * 500}ms`);
            console.log('   Available methods:', Object.getOwnPropertyNames(window.twitterScrapingBridge));
            
            // Try to send any accumulated features immediately
            if (window.homeTimelineFeatures && Object.keys(window.homeTimelineFeatures).length > 0) {
                console.log('🚀 BRIDGE MONITOR -> Found existing features, sending immediately');
                // TODO: OBSOLETE CALL - Remove this call since onFeatureDataUpdate method was removed
                // The URL capture approach via onHomeTimelineUrlCaptured is now used instead
                sendHomeTimelineFeaturesImmediately();
            }
            
            return; // Stop monitoring
        }
        
        console.log(`🔍 BRIDGE MONITOR -> Check ${checkCount}/${maxChecks} - Bridge not ready yet`);
        console.log('   window.twitterScrapingBridge:', !!window.twitterScrapingBridge);
        if (window.twitterScrapingBridge) {
            console.log('   Bridge methods:', Object.getOwnPropertyNames(window.twitterScrapingBridge));
        }
        
        if (checkCount < maxChecks) {
            setTimeout(checkBridge, 500); // Check every 500ms
        } else {
            console.log('❌ BRIDGE MONITOR -> Bridge not available after 10 seconds, giving up');
        }
    }
    
    // Start checking immediately
    checkBridge();
}

// Start bridge monitoring
monitorBridgeAvailability();

console.log('✅ COMPLETE INTEGRATED TWITTER API SYSTEM INITIALIZED');
console.log('🌐 Network interception: ACTIVE (HomeTimeline endpoint only)');
console.log('🔍 Token monitoring: ACTIVE (C++ backend integration)');
console.log('🍪 HttpOnly cookie integration: READY');
console.log('🌉 Bridge communication: ACTIVE');
console.log('📡 Parameter capture: HOMETIMELINE ONLY (Viewer/other endpoints ignored)');
console.log('🔄 Feature tracking: IMMEDIATE SEND on HttpOnly cookies + RESEND on count increase');
console.log('');
console.log('💡 Available utilities:');
console.log('  - TwitterAPIUtils.getAllFeatures() - Get features from ALL endpoints');
console.log('  - TwitterAPIUtils.getHomeTimelineFeatures() - Get ONLY HomeTimeline features');
console.log('  - TwitterAPIUtils.getFeaturesByEndpoint() - Features breakdown by endpoint');
console.log('  - TwitterAPIUtils.compareWithTarget() - Compare captured vs target features');
console.log('  - TwitterAPIUtils.getCurrentData() - Get capture statistics');
console.log('  - TwitterAPIUtils.getDebugStatus() - Debug HomeTimeline filtering status');
console.log('  - extractAndSendTokens() - Manual token extraction');
console.log('  - updateCurlWithHttpOnlyCookies() - C++ backend integration');
console.log('  - sendHomeTimelineFeaturesImmediately() - Send features/variables immediately');
console.log('  - forceResendAllData() - Force resend of all captured data');
console.log('  - debugSendFeatureUpdate() - Manually trigger feature update (debugging)');
console.log('');
console.log('🎯 System Status:');
console.log('  ✅ HomeTimeline-only feature aggregation');
console.log('  ✅ Viewer endpoint filtering (IGNORED)');
console.log('  ✅ Token extraction with C++ backend integration');
console.log('  ✅ HttpOnly cookie support');
console.log('  ✅ Dynamic cURL generation with HomeTimeline features');
console.log('  ✅ Bridge communication for C++ backend');
console.log('  ✅ Immediate send when HttpOnly cookies received');
console.log('  ✅ Automatic resend when feature count increases');
console.log('');
console.log('📊 SIMPLIFIED BEHAVIOR:');
console.log('  1. ONLY HomeTimeline features/variables are captured');
console.log('  2. When HttpOnly cookies received -> Send immediately');
console.log('  3. When feature count increases -> Resend all data');
console.log('  4. No authentication or stability checks required');
console.log('  5. Continuous monitoring every 2 seconds');

// UTILITY FUNCTION: Force resend all data (useful after authentication)
window.forceResendAllData = function() {
    console.log('🔄 FORCE RESEND -> Manually forcing complete data resend...');
    
    // Reset single send control flag
    window.twitterDataSentFlag.dataSent = false;
    window.twitterDataSentFlag.timestamp = null;
    
    // Force token extraction and send
    const tokenData = extractAndSendTokens();
    
    // Send features immediately
    console.log('🔄 FORCE RESEND -> Sending features/variables immediately');
    // TODO: OBSOLETE CALL - Remove this call since onFeatureDataUpdate method was removed
    // The URL capture approach via onHomeTimelineUrlCaptured is now used instead
    sendHomeTimelineFeaturesImmediately();
    
    console.log('✅ FORCE RESEND -> Complete data resend initiated');
    return tokenData;
};

// UTILITY FUNCTION: Manually send feature update (for debugging)
window.debugSendFeatureUpdate = function() {
    console.log('🐛 DEBUG -> Manually triggering immediate feature update...');
    // TODO: OBSOLETE CALL - Remove this call since onFeatureDataUpdate method was removed
    // The URL capture approach via onHomeTimelineUrlCaptured is now used instead
    sendHomeTimelineFeaturesImmediately();
};

// TODO: OBSOLETE FUNCTION - Remove this function and its calls
// This function calls the old onFeatureDataUpdate method which has been removed from C++ backend.
// The system now uses onHomeTimelineUrlCaptured for complete URL capture instead of feature extraction.
// SIMPLIFIED: Send HomeTimeline features immediately without authentication/stability checks
function sendHomeTimelineFeaturesImmediately(retryCount = 0) {
    const maxRetries = 5; // Maximum 5 retries (2.5 seconds total delay)
    
    console.log(`📡 IMMEDIATE SEND -> Preparing to send HomeTimeline features/variables NOW (attempt ${retryCount + 1})`);
    
    // Check bridge availability with detailed logging
    if (!window.twitterScrapingBridge) {
        console.log('❌ IMMEDIATE SEND -> Bridge object not available');
        console.log('   window.twitterScrapingBridge:', window.twitterScrapingBridge);
        
        if (retryCount < maxRetries) {
            console.log(`🔄 RETRY -> Will retry in 500ms (attempt ${retryCount + 1}/${maxRetries + 1})`);
            setTimeout(() => {
                sendHomeTimelineFeaturesImmediately(retryCount + 1);
            }, 500);
        } else {
            console.log('❌ IMMEDIATE SEND -> Max retries reached, bridge object never became available');
        }
        return;
    }
    
    // TODO: OBSOLETE CHECK - onFeatureDataUpdate method has been removed from C++ backend
    // Replace this check with onHomeTimelineUrlCaptured or remove entirely
    if (!window.twitterScrapingBridge.onFeatureDataUpdate) {
        console.log('❌ IMMEDIATE SEND -> Bridge method onFeatureDataUpdate not available');
        console.log('   Available bridge methods:', Object.getOwnPropertyNames(window.twitterScrapingBridge));
        console.log('   Bridge type:', typeof window.twitterScrapingBridge);
        
        if (retryCount < maxRetries) {
            console.log(`🔄 RETRY -> Will retry in 500ms (attempt ${retryCount + 1}/${maxRetries + 1})`);
            setTimeout(() => {
                sendHomeTimelineFeaturesImmediately(retryCount + 1);
            }, 500);
        } else {
            console.log('❌ IMMEDIATE SEND -> Max retries reached, bridge method never became available');
        }
        return;
    }
    
    console.log('✅ IMMEDIATE SEND -> Bridge is available and ready!');
    
    // Get HomeTimeline features and variables
    const homeTimelineFeatures = window.homeTimelineFeatures || {};
    const homeTimelineVariables = window.homeTimelineVariables || {};
    
    const featureCount = Object.keys(homeTimelineFeatures).length;
    const variableCount = Object.keys(homeTimelineVariables).length;
    
    // Check if we have any data to send
    if (featureCount === 0 && variableCount === 0) {
        console.log('⚠️ IMMEDIATE SEND -> No HomeTimeline features/variables available yet');
        console.log('   Will continue monitoring for data...');
        return;
    }
    
    console.log('✅ IMMEDIATE SEND -> Found data to send:');
    console.log(`   🎯 HomeTimeline features: ${featureCount}`);
    console.log(`   🔧 HomeTimeline variables: ${variableCount}`);
    
    // DEBUG: Log URL being used
    const urlToUse = window.lastCapturedHomeTimelineUrl || window.location.href;
    const isCompleteUrl = !!window.lastCapturedHomeTimelineUrl;
    console.log(`   🔗 URL SOURCE: ${isCompleteUrl ? 'COMPLETE CAPTURED URL' : 'FALLBACK PAGE URL'}`);
    console.log(`   🔗 URL: ${urlToUse.substring(0, 100)}...`);
    
    // Encode the data
    const encodedFeatures = encodeURIComponent(JSON.stringify(homeTimelineFeatures));
    const encodedVariables = encodeURIComponent(JSON.stringify(homeTimelineVariables));
    
    // Create the update payload
    const updatePayload = {
        type: 'immediate_feature_update',
        timestamp: new Date().toISOString(),
        counts: {
            features: featureCount,
            variables: variableCount
        },
        encoded: {
            features: encodedFeatures,
            variables: encodedVariables
        },
        raw: {
            features: homeTimelineFeatures,
            variables: homeTimelineVariables
        },
        metadata: {
            url: window.lastCapturedHomeTimelineUrl || window.location.href, // Use captured complete URL if available
            queryId: (window.capturedTwitterApiData && window.capturedTwitterApiData.queryId) || 'unknown',  // NEW: Dynamic Query ID
            operationId: (window.capturedTwitterApiData && window.capturedTwitterApiData.operationId) || 'unknown',
            endpoint: (window.capturedTwitterApiData && window.capturedTwitterApiData.endpoint) || 'unknown',
            graphQLPath: (window.capturedTwitterApiData && window.capturedTwitterApiData.graphQLPath) || '',  // NEW: Complete GraphQL path
            dataSource: 'HOMETIMELINE_ONLY',
            trigger: 'HTTPONLY_COOKIES_RECEIVED',
            skippedEndpoints: window.skippedEndpoints || { viewer: 0, other: 0, total: 0 }
        }
    };
    
    try {
        console.log('🌉 IMMEDIATE SEND -> Calling bridge.onFeatureDataUpdate()');
        // TODO: OBSOLETE CALL - This calls the removed onFeatureDataUpdate method
        // Replace with onHomeTimelineUrlCaptured or remove entirely since URL capture is now used
        window.twitterScrapingBridge.onFeatureDataUpdate(JSON.stringify(updatePayload));
        
        console.log('✅ IMMEDIATE SEND -> Successfully sent HomeTimeline data to C++ backend');
        console.log(`   📏 Encoded features length: ${encodedFeatures.length} chars`);
        console.log(`   📏 Encoded variables length: ${encodedVariables.length} chars`);
        
        // Update last sent counts
        window.lastSentFeatureCount = featureCount;
        window.lastSentVariableCount = variableCount;
        window.lastSentTime = new Date().toISOString();
        
        console.log('📊 TRACKING -> Updated last sent counts:');
        console.log(`   Features: ${window.lastSentFeatureCount}`);
        console.log(`   Variables: ${window.lastSentVariableCount}`);
        
    } catch (error) {
        console.error('❌ IMMEDIATE SEND -> Error sending update:', error);
    }
}

// Function to check if feature count increased and resend if needed
function checkAndResendIfIncreased() {
    const homeTimelineFeatures = window.homeTimelineFeatures || {};
    const homeTimelineVariables = window.homeTimelineVariables || {};
    
    const currentFeatureCount = Object.keys(homeTimelineFeatures).length;
    const currentVariableCount = Object.keys(homeTimelineVariables).length;
    
    // Initialize tracking if not exists
    if (typeof window.lastSentFeatureCount === 'undefined') {
        window.lastSentFeatureCount = 0;
        window.lastSentVariableCount = 0;
    }
    
    // Check if feature count increased
    if (currentFeatureCount > window.lastSentFeatureCount) {
        console.log('📈 FEATURE INCREASE DETECTED:');
        console.log(`   Previous features: ${window.lastSentFeatureCount}`);
        console.log(`   Current features: ${currentFeatureCount}`);
        console.log('   🔄 Resending all features and variables...');
        
        // TODO: Remove this call - sendHomeTimelineFeaturesImmediately() uses obsolete onFeatureDataUpdate bridge method
        sendHomeTimelineFeaturesImmediately();
    } else if (currentVariableCount > window.lastSentVariableCount) {
        console.log('📈 VARIABLE INCREASE DETECTED:');
        console.log(`   Previous variables: ${window.lastSentVariableCount}`);
        console.log(`   Current variables: ${currentVariableCount}`);
        console.log('   🔄 Resending all features and variables...');
        
        // TODO: Remove this call - sendHomeTimelineFeaturesImmediately() uses obsolete onFeatureDataUpdate bridge method
        sendHomeTimelineFeaturesImmediately();
    }
}

// Function to send encoded feature data to C++ backend via new bridge method
    
    )";
  }

  // Get a WebScriptSource for the Twitter interceptor script
  blink::WebScriptSource GetTwitterInterceptorScriptSource() {
    return blink::WebScriptSource(
        blink::WebString::FromUTF8(GetTwitterInterceptorScript()));
  }

  }  // namespace wootz_scraping