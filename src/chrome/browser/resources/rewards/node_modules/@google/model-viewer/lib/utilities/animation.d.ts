export declare const easeInOutQuad: TimingFunction;
/**
 * A TimingFunction accepts a value from 0-1 and returns a corresponding
 * interpolated value
 */
export declare type TimingFunction = (time: number) => number;
/**
 * Creates a TimingFunction that uses a given ease to interpolate between
 * two configured number values.
 */
export declare const interpolate: (start: number, end: number, ease?: TimingFunction) => TimingFunction;
/**
 * Creates a TimingFunction that interpolates through a weighted list
 * of other TimingFunctions ("tracks"). Tracks are interpolated in order, and
 * allocated a percentage of the total time based on their relative weight.
 */
export declare const sequence: (tracks: Array<TimingFunction>, weights: Array<number>) => TimingFunction;
/**
 * A Frame groups a target value, the number of frames to interpolate towards
 * that value and an optional easing function to use for interpolation.
 */
export interface Frame {
    value: number;
    frames: number;
    ease?: TimingFunction;
}
export interface Path {
    initialValue: number;
    keyframes: Frame[];
}
/**
 * Creates a "timeline" TimingFunction out of an initial value and a series of
 * Keyframes. The timeline function accepts value from 0-1 and returns the
 * current value based on keyframe interpolation across the total number of
 * frames. Frames are only used to indicate the relative length of each keyframe
 * transition, so interpolated values will be computed for fractional frames.
 */
export declare const timeline: (path: Path) => TimingFunction;
