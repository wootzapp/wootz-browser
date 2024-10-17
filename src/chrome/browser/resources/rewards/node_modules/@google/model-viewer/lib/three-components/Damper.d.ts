export declare const SETTLING_TIME = 10000;
export declare const DECAY_MILLISECONDS = 50;
/**
 * The Damper class is a generic second-order critically damped system that does
 * one linear step of the desired length of time. The only parameter is
 * DECAY_MILLISECONDS. This common parameter makes all states converge at the
 * same rate regardless of scale. xNormalization is a number to provide the
 * rough scale of x, such that NIL_SPEED clamping also happens at roughly the
 * same convergence for all states.
 */
export declare class Damper {
    private velocity;
    private naturalFrequency;
    constructor(decayMilliseconds?: number);
    setDecayTime(decayMilliseconds: number): void;
    update(x: number, xGoal: number, timeStepMilliseconds: number, xNormalization: number): number;
}
