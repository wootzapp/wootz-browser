/* @license
 * Copyright 2019 Google LLC. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { HAS_WEBXR_DEVICE_API, HAS_WEBXR_HIT_TEST_API, IS_WEBXR_AR_CANDIDATE } from './constants.js';
export const deserializeUrl = (url) => (!!url && url !== 'null') ? toFullUrl(url) : null;
export const assertIsArCandidate = () => {
    if (IS_WEBXR_AR_CANDIDATE) {
        return;
    }
    const missingApis = [];
    if (!HAS_WEBXR_DEVICE_API) {
        missingApis.push('WebXR Device API');
    }
    if (!HAS_WEBXR_HIT_TEST_API) {
        missingApis.push('WebXR Hit Test API');
    }
    throw new Error(`The following APIs are required for AR, but are missing in this browser: ${missingApis.join(', ')}`);
};
/**
 * Converts a partial URL string to a fully qualified URL string.
 *
 * @param {String} url
 * @return {String}
 */
export const toFullUrl = (partialUrl) => {
    const url = new URL(partialUrl, window.location.toString());
    return url.toString();
};
/**
 * Returns a throttled version of a given function that is only invoked at most
 * once within a given threshold of time in milliseconds.
 *
 * The throttled version of the function has a "flush" property that resets the
 * threshold for cases when immediate invocation is desired.
 */
export const throttle = (fn, ms) => {
    let timer = null;
    const throttled = (...args) => {
        if (timer != null) {
            return;
        }
        fn(...args);
        timer = self.setTimeout(() => timer = null, ms);
    };
    throttled.flush = () => {
        if (timer != null) {
            self.clearTimeout(timer);
            timer = null;
        }
    };
    return throttled;
};
export const debounce = (fn, ms) => {
    let timer = null;
    return (...args) => {
        if (timer != null) {
            self.clearTimeout(timer);
        }
        timer = self.setTimeout(() => {
            timer = null;
            fn(...args);
        }, ms);
    };
};
/**
 * @param {Number} edge
 * @param {Number} value
 * @return {Number} 0 if value is less than edge, otherwise 1
 */
export const step = (edge, value) => {
    return value < edge ? 0 : 1;
};
/**
 * @param {Number} value
 * @param {Number} lowerLimit
 * @param {Number} upperLimit
 * @return {Number} value clamped within lowerLimit..upperLimit
 */
export const clamp = (value, lowerLimit, upperLimit) => Math.max(lowerLimit, Math.min(upperLimit, value));
// The DPR we use for a "capped" scenario (see resolveDpr below):
export const CAPPED_DEVICE_PIXEL_RATIO = 1;
/**
 * This helper analyzes the layout of the current page to decide if we should
 * use the natural device pixel ratio, or a capped value.
 *
 * We cap DPR if there is no meta viewport (suggesting that user is not
 * consciously specifying how to scale the viewport relative to the device
 * screen size).
 *
 * The rationale is that this condition typically leads to a pathological
 * outcome on mobile devices. When the window dimensions are scaled up on a
 * device with a high DPR, we create a canvas that is much larger than
 * appropriate to accommodate for the pixel density if we naively use the
 * reported DPR.
 *
 * This value needs to be measured in real time, as device pixel ratio can
 * change over time (e.g., when a user zooms the page). Also, in some cases
 * (such as Firefox on Android), the window's innerWidth is initially reported
 * as the same as the screen's availWidth but changes later.
 *
 * A user who specifies a meta viewport, thereby consciously creating scaling
 * conditions where <model-viewer> is slow, will be encouraged to live their
 * best life.
 */
export const resolveDpr = (() => {
    // If true, implies that the user is conscious of the viewport scaling
    // relative to the device screen size.
    const HAS_META_VIEWPORT_TAG = (() => {
        var _a;
        // Search result pages sometimes do not include a meta viewport tag even
        // though they are certainly modern and work properly with devicePixelRatio.
        if ((_a = document.documentElement.getAttribute('itemtype')) === null || _a === void 0 ? void 0 : _a.includes('schema.org/SearchResultsPage')) {
            return true;
        }
        const metas = document.head != null ?
            Array.from(document.head.querySelectorAll('meta')) :
            [];
        for (const meta of metas) {
            if (meta.name === 'viewport') {
                return true;
            }
        }
        return false;
    })();
    if (!HAS_META_VIEWPORT_TAG) {
        console.warn('No <meta name="viewport"> detected; <model-viewer> will cap pixel density at 1.');
    }
    return () => HAS_META_VIEWPORT_TAG ? window.devicePixelRatio :
        CAPPED_DEVICE_PIXEL_RATIO;
})();
/**
 * Debug mode is enabled when one of the two following conditions is true:
 *
 *  1. A 'model-viewer-debug-mode' query parameter is present in the current
 *     search string
 *  2. There is a global object ModelViewerElement with a debugMode property set
 *     to true
 */
export const isDebugMode = (() => {
    const debugQueryParameterName = 'model-viewer-debug-mode';
    const debugQueryParameter = new RegExp(`[?&]${debugQueryParameterName}(&|$)`);
    return () => (self.ModelViewerElement &&
        self.ModelViewerElement.debugMode) ||
        (self.location && self.location.search &&
            self.location.search.match(debugQueryParameter));
})();
/**
 * Returns the first key in a Map in iteration order.
 *
 * NOTE(cdata): This is necessary because IE11 does not implement iterator
 * methods of Map, and polymer-build does not polyfill these methods for
 * compatibility and performance reasons. This helper proposes that it is
 * a reasonable compromise to sacrifice a very small amount of runtime
 * performance in IE11 for the sake of code clarity.
 */
export const getFirstMapKey = (map) => {
    if (map.keys != null) {
        return map.keys().next().value || null;
    }
    let firstKey = null;
    try {
        map.forEach((_value, key, _map) => {
            firstKey = key;
            // Stop iterating the Map with forEach:
            throw new Error();
        });
    }
    catch (_error) {
    }
    return firstKey;
};
export const timePasses = (ms = 0) => new Promise(resolve => setTimeout(resolve, ms));
/**
 * @param {EventTarget|EventDispatcher} target
 * @param {string} eventName
 * @param {?Function} predicate
 */
export const waitForEvent = (target, eventName, predicate = null) => new Promise(resolve => {
    function handler(event) {
        if (!predicate || predicate(event)) {
            resolve(event);
            target.removeEventListener(eventName, handler);
        }
    }
    target.addEventListener(eventName, handler);
});
//# sourceMappingURL=utilities.js.map