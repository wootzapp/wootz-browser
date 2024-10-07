/**
 * An Activity is represented by a callback that accepts values from 0 to 1,
 * where 1 represents the completion of the activity. The callback returns the
 * actual progress as it is stored by the ProgressTracker (which may be clamped,
 * and can never be lower than its previous value).
 */
export declare type Activity = (progress: number) => number;
/**
 * A progress event contains the total progress of all ongoing activities in the
 * ProgressTracker. The total progress is a heuristic, but has some useful
 * properties: for a single activity, it equals the input progress; for multiple
 * activities that progress in lockstep, it will also equal each input progress.
 * When more activities overlap as time goes on, total progress will tend to
 * decelerate.
 */
export interface ProgressDetails {
    totalProgress: number;
}
/**
 * ProgressTracker is an event emitter that helps to track the ongoing progress
 * of many simultaneous actions.
 *
 * ProgressTracker reports progress activity in the form of a progress event.
 * The event.detail.totalProgress value indicates the elapsed progress of all
 * activities being tracked by the ProgressTracker.
 *
 * The value of totalProgress is a number that progresses from 0 to 1. The
 * ProgressTracker allows for the lazy accumulation of tracked actions, so the
 * total progress represents a abstract, non-absolute progress towards the
 * completion of all currently tracked events.
 *
 * When all currently tracked activities are finished, the ProgressTracker
 * emits one final progress event and then resets the list of its currently
 * tracked activities. This means that from an observer's perspective,
 * ongoing activities will accumulate and collectively contribute to the notion
 * of total progress until all currently tracked ongoing activities have
 * completed.
 */
export declare class ProgressTracker extends EventTarget {
    private ongoingActivities;
    private totalProgress;
    /**
     * The total number of activities currently being tracked.
     */
    get ongoingActivityCount(): number;
    /**
     * Registers a new activity to be tracked by the progress tracker. The method
     * returns a special callback that should be invoked whenever new progress is
     * ready to be reported. The progress should be reported as a value between 0
     * and 1, where 0 would represent the beginning of the action and 1 would
     * represent its completion.
     *
     * There is no built-in notion of a time-out for ongoing activities, so once
     * an ongoing activity is begun, it is up to the consumer of this API to
     * update the progress until that activity is no longer ongoing.
     *
     * Progress is only allowed to move forward for any given activity. If a lower
     * progress is reported than the previously reported progress, it will be
     * ignored.
     */
    beginActivity(): Activity;
    private announceTotalProgress;
}
