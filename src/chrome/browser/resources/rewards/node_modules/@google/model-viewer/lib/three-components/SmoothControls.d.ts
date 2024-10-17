import { Event as ThreeEvent, EventDispatcher, PerspectiveCamera, Spherical } from 'three';
import { TouchAction } from '../features/controls.js';
import { ModelScene } from './ModelScene.js';
export declare type TouchMode = null | ((dx: number, dy: number) => void);
export interface Pointer {
    clientX: number;
    clientY: number;
    id: number;
}
export interface SmoothControlsOptions {
    minimumRadius?: number;
    maximumRadius?: number;
    minimumPolarAngle?: number;
    maximumPolarAngle?: number;
    minimumAzimuthalAngle?: number;
    maximumAzimuthalAngle?: number;
    minimumFieldOfView?: number;
    maximumFieldOfView?: number;
    touchAction?: TouchAction;
}
export declare const DEFAULT_OPTIONS: Readonly<SmoothControlsOptions>;
export declare const KeyCode: {
    PAGE_UP: number;
    PAGE_DOWN: number;
    LEFT: number;
    UP: number;
    RIGHT: number;
    DOWN: number;
};
export declare type ChangeSource = 'user-interaction' | 'none' | 'automatic';
export declare const ChangeSource: {
    [index: string]: ChangeSource;
};
/**
 * ChangEvents are dispatched whenever the camera position or orientation has
 * changed
 */
export interface ChangeEvent extends ThreeEvent {
    /**
     * determines what was the originating reason for the change event eg user or
     * none
     */
    source: ChangeSource;
}
export interface PointerChangeEvent extends ThreeEvent {
    type: 'pointer-change-start' | 'pointer-change-end';
}
/**
 * SmoothControls is a Three.js helper for adding delightful pointer and
 * keyboard-based input to a staged Three.js scene. Its API is very similar to
 * OrbitControls, but it offers more opinionated (subjectively more delightful)
 * defaults, easy extensibility and subjectively better out-of-the-box keyboard
 * support.
 *
 * One important change compared to OrbitControls is that the `update` method
 * of SmoothControls must be invoked on every frame, otherwise the controls
 * will not have an effect.
 *
 * Another notable difference compared to OrbitControls is that SmoothControls
 * does not currently support panning (but probably will in a future revision).
 *
 * Like OrbitControls, SmoothControls assumes that the orientation of the camera
 * has been set in terms of position, rotation and scale, so it is important to
 * ensure that the camera's matrixWorld is in sync before using SmoothControls.
 */
export declare class SmoothControls extends EventDispatcher {
    readonly camera: PerspectiveCamera;
    readonly element: HTMLElement;
    readonly scene: ModelScene;
    orbitSensitivity: number;
    inputSensitivity: number;
    changeSource: ChangeSource;
    private _interactionEnabled;
    private _options;
    private _disableZoom;
    private isUserPointing;
    enablePan: boolean;
    enableTap: boolean;
    private panProjection;
    private panPerPixel;
    private spherical;
    private goalSpherical;
    private thetaDamper;
    private phiDamper;
    private radiusDamper;
    private logFov;
    private goalLogFov;
    private fovDamper;
    private touchMode;
    private pointers;
    private startTime;
    private startPointerPosition;
    private lastSeparation;
    private touchDecided;
    constructor(camera: PerspectiveCamera, element: HTMLElement, scene: ModelScene);
    get interactionEnabled(): boolean;
    enableInteraction(): void;
    disableInteraction(): void;
    /**
     * The options that are currently configured for the controls instance.
     */
    get options(): SmoothControlsOptions;
    onContext: (event: MouseEvent) => void;
    set disableZoom(disable: boolean);
    /**
     * Copy the spherical values that represent the current camera orbital
     * position relative to the configured target into a provided Spherical
     * instance. If no Spherical is provided, a new Spherical will be allocated
     * to copy the values into. The Spherical that values are copied into is
     * returned.
     */
    getCameraSpherical(target?: Spherical): Spherical;
    /**
     * Returns the camera's current vertical field of view in degrees.
     */
    getFieldOfView(): number;
    /**
     * Configure the _options of the controls. Configured _options will be
     * merged with whatever _options have already been configured for this
     * controls instance.
     */
    applyOptions(_options: SmoothControlsOptions): void;
    /**
     * Sets the near and far planes of the camera.
     */
    updateNearFar(nearPlane: number, farPlane: number): void;
    /**
     * Sets the aspect ratio of the camera
     */
    updateAspect(aspect: number): void;
    /**
     * Set the absolute orbital goal of the camera. The change will be
     * applied over a number of frames depending on configured acceleration and
     * dampening _options.
     *
     * Returns true if invoking the method will result in the camera changing
     * position and/or rotation, otherwise false.
     */
    setOrbit(goalTheta?: number, goalPhi?: number, goalRadius?: number): boolean;
    /**
     * Subset of setOrbit() above, which only sets the camera's radius.
     */
    setRadius(radius: number): void;
    /**
     * Sets the goal field of view for the camera
     */
    setFieldOfView(fov: number): void;
    /**
     * Sets the smoothing decay time.
     */
    setDamperDecayTime(decayMilliseconds: number): void;
    /**
     * Adjust the orbital position of the camera relative to its current orbital
     * position. Does not let the theta goal get more than pi ahead of the current
     * theta, which ensures interpolation continues in the direction of the delta.
     * The deltaZoom parameter adjusts both the field of view and the orbit radius
     * such that they progress across their allowed ranges in sync.
     */
    adjustOrbit(deltaTheta: number, deltaPhi: number, deltaZoom: number): void;
    /**
     * Move the camera instantly instead of accelerating toward the goal
     * parameters.
     */
    jumpToGoal(): void;
    /**
     * Update controls. In most cases, this will result in the camera
     * interpolating its position and rotation until it lines up with the
     * designated goal orbital position.
     *
     * Time and delta are measured in milliseconds.
     */
    update(_time: number, delta: number): void;
    updateTouchActionStyle(): void;
    private isStationary;
    private dispatchChange;
    private moveCamera;
    private userAdjustOrbit;
    private wrapAngle;
    private pixelLengthToSphericalAngle;
    private twoTouchDistance;
    private touchModeZoom;
    private disableScroll;
    private touchModeRotate;
    private handleSinglePointerMove;
    private initializePan;
    private movePan;
    private recenter;
    private resetRadius;
    private onPointerDown;
    private onPointerMove;
    private onPointerUp;
    private onTouchChange;
    private onMouseDown;
    private onWheel;
    private onKeyDown;
    /**
     * Handles the orbit and Zoom key presses
     * Uses constants for the increment.
     * @param event The keyboard event for the .key value
     * @returns boolean to indicate if the key event has been handled
     */
    private orbitZoomKeyCodeHandler;
    /**
     * Handles the Pan key presses
     * Uses constants for the increment.
     * @param event The keyboard event for the .key value
     * @returns boolean to indicate if the key event has been handled
     */
    private panKeyCodeHandler;
}
