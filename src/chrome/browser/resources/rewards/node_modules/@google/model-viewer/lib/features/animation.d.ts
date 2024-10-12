import ModelViewerElementBase from '../model-viewer-base.js';
import { Constructor } from '../utilities.js';
interface PlayAnimationOptions {
    repetitions: number;
    pingpong: boolean;
}
export declare interface AnimationInterface {
    autoplay: boolean;
    animationName: string | void;
    animationCrossfadeDuration: number;
    readonly availableAnimations: Array<string>;
    readonly paused: boolean;
    readonly duration: number;
    currentTime: number;
    timeScale: number;
    pause(): void;
    play(options?: PlayAnimationOptions): void;
}
export declare const AnimationMixin: <T extends Constructor<ModelViewerElementBase, object>>(ModelViewerElement: T) => {
    new (...args: any[]): AnimationInterface;
    prototype: AnimationInterface;
} & object & T;
export {};
