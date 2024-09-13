/// <reference types="webxr" />
import { Event as ThreeEvent, EventDispatcher, Vector3, WebGLRenderer } from 'three';
import { ModelScene } from './ModelScene.js';
import { Renderer } from './Renderer.js';
export declare type ARStatus = 'not-presenting' | 'session-started' | 'object-placed' | 'failed';
export declare const ARStatus: {
    [index: string]: ARStatus;
};
export interface ARStatusEvent extends ThreeEvent {
    status: ARStatus;
}
export declare type ARTracking = 'tracking' | 'not-tracking';
export declare const ARTracking: {
    [index: string]: ARTracking;
};
export interface ARTrackingEvent extends ThreeEvent {
    status: ARTracking;
}
export declare class ARRenderer extends EventDispatcher {
    private renderer;
    threeRenderer: WebGLRenderer;
    currentSession: XRSession | null;
    placeOnWall: boolean;
    private placementBox;
    private lastTick;
    private turntableRotation;
    private oldShadowIntensity;
    private frame;
    private initialHitSource;
    private transientHitTestSource;
    private inputSource;
    private _presentedScene;
    private resolveCleanup;
    private exitWebXRButtonContainer;
    private overlay;
    private xrLight;
    private tracking;
    private frames;
    private initialized;
    private oldTarget;
    private placementComplete;
    private isTranslating;
    private isRotating;
    private isTwoFingering;
    private lastDragPosition;
    private firstRatio;
    private lastAngle;
    private goalPosition;
    private goalYaw;
    private goalScale;
    private xDamper;
    private yDamper;
    private zDamper;
    private yawDamper;
    private scaleDamper;
    private onExitWebXRButtonContainerClick;
    constructor(renderer: Renderer);
    resolveARSession(): Promise<XRSession>;
    /**
     * The currently presented scene, if any
     */
    get presentedScene(): ModelScene | null;
    /**
     * Resolves to true if the renderer has detected all the necessary qualities
     * to support presentation in AR.
     */
    supportsPresentation(): Promise<boolean>;
    /**
     * Present a scene in AR
     */
    present(scene: ModelScene, environmentEstimation?: boolean): Promise<void>;
    /**
     * If currently presenting a scene in AR, stops presentation and exits AR.
     */
    stopPresenting(): Promise<void>;
    /**
     * True if a scene is currently in the process of being presented in AR
     */
    get isPresenting(): boolean;
    get target(): Vector3;
    updateTarget(): void;
    onUpdateScene: () => void;
    private postSessionCleanup;
    private updateView;
    private placeInitially;
    private getTouchLocation;
    private getHitPoint;
    moveToFloor(frame: XRFrame): void;
    private onSelectStart;
    private onSelectEnd;
    private fingerPolar;
    private processInput;
    private moveScene;
    /**
     * Only public to make it testable.
     */
    onWebXRFrame(time: number, frame: XRFrame): void;
}
