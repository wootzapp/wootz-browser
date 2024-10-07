import { EventDispatcher } from 'three';
export declare type Constructor<T = object, U = object> = {
    new (...args: any[]): T;
    prototype: T;
} & U;
export declare const deserializeUrl: (url: string | null) => string | null;
export declare const assertIsArCandidate: () => void;
/**
 * Converts a partial URL string to a fully qualified URL string.
 *
 * @param {String} url
 * @return {String}
 */
export declare const toFullUrl: (partialUrl: string) => string;
/**
 * Returns a throttled version of a given function that is only invoked at most
 * once within a given threshold of time in milliseconds.
 *
 * The throttled version of the function has a "flush" property that resets the
 * threshold for cases when immediate invocation is desired.
 */
export declare const throttle: (fn: (...args: Array<any>) => any, ms: number) => {
    (...args: Array<any>): void;
    flush(): void;
};
export declare const debounce: (fn: (...args: Array<any>) => any, ms: number) => (...args: Array<any>) => void;
/**
 * @param {Number} edge
 * @param {Number} value
 * @return {Number} 0 if value is less than edge, otherwise 1
 */
export declare const step: (edge: number, value: number) => number;
/**
 * @param {Number} value
 * @param {Number} lowerLimit
 * @param {Number} upperLimit
 * @return {Number} value clamped within lowerLimit..upperLimit
 */
export declare const clamp: (value: number, lowerLimit: number, upperLimit: number) => number;
export declare const CAPPED_DEVICE_PIXEL_RATIO = 1;
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
export declare const resolveDpr: () => number;
/**
 * Debug mode is enabled when one of the two following conditions is true:
 *
 *  1. A 'model-viewer-debug-mode' query parameter is present in the current
 *     search string
 *  2. There is a global object ModelViewerElement with a debugMode property set
 *     to true
 */
export declare const isDebugMode: () => any;
/**
 * Returns the first key in a Map in iteration order.
 *
 * NOTE(cdata): This is necessary because IE11 does not implement iterator
 * methods of Map, and polymer-build does not polyfill these methods for
 * compatibility and performance reasons. This helper proposes that it is
 * a reasonable compromise to sacrifice a very small amount of runtime
 * performance in IE11 for the sake of code clarity.
 */
export declare const getFirstMapKey: <T = any, U = any>(map: Map<T, U>) => T | null;
/**
 * Three.js EventDispatcher and DOM EventTarget use different event patterns,
 * so AnyEvent covers the shape of both event types.
 */
export declare type AnyEvent = Event | CustomEvent<any> | {
    [index: string]: string;
};
export declare type PredicateFunction<T = void> = (value: T) => boolean;
export declare const timePasses: (ms?: number) => Promise<void>;
/**
 * @param {EventTarget|EventDispatcher} target
 * @param {string} eventName
 * @param {?Function} predicate
 */
export declare const waitForEvent: <T extends AnyEvent = Event>(target: EventTarget | EventDispatcher, eventName: string, predicate?: PredicateFunction<T> | null) => Promise<T>;
