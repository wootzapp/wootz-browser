import ModelViewerElementBase, { Vector3D } from '../model-viewer-base.js';
import { NumberNode } from '../styles/parsers.js';
import { ChangeSource } from '../three-components/SmoothControls.js';
import { Constructor } from '../utilities.js';
import { Path } from '../utilities/animation.js';
export declare const DEFAULT_FOV_DEG = 30;
export declare const DEFAULT_MIN_FOV_DEG = 12;
export declare const DEFAULT_CAMERA_ORBIT = "0deg 75deg 105%";
export declare const DEFAULT_INTERACTION_PROMPT_THRESHOLD = 3000;
export declare const INTERACTION_PROMPT = ". Use mouse, touch or arrow keys to move.";
export interface CameraChangeDetails {
    source: ChangeSource;
}
export interface SphericalPosition {
    theta: number;
    phi: number;
    radius: number;
    toString(): string;
}
export interface Finger {
    x: Path;
    y: Path;
}
export declare type InteractionPromptStrategy = 'auto' | 'none';
export declare type InteractionPromptStyle = 'basic' | 'wiggle';
export declare type TouchAction = 'pan-y' | 'pan-x' | 'none';
export declare const InteractionPromptStrategy: {
    [index: string]: InteractionPromptStrategy;
};
export declare const InteractionPromptStyle: {
    [index: string]: InteractionPromptStyle;
};
export declare const TouchAction: {
    [index: string]: TouchAction;
};
export declare const fieldOfViewIntrinsics: () => {
    basis: NumberNode<"rad">[];
    keywords: {
        auto: null[];
    };
};
export declare const cameraOrbitIntrinsics: (element: ModelViewerElementBase) => {
    basis: (NumberNode<"rad"> | NumberNode<"m">)[];
    keywords: {
        auto: (NumberNode<"%"> | null)[];
    };
};
export declare const cameraTargetIntrinsics: (element: ModelViewerElementBase) => {
    basis: NumberNode<"m">[];
    keywords: {
        auto: null[];
    };
};
export declare const $controls: unique symbol;
export declare const $panElement: unique symbol;
export declare const $promptElement: unique symbol;
export declare const $promptAnimatedContainer: unique symbol;
export declare const $fingerAnimatedContainers: unique symbol;
export declare interface ControlsInterface {
    cameraControls: boolean;
    cameraOrbit: string;
    cameraTarget: string;
    fieldOfView: string;
    minCameraOrbit: string;
    maxCameraOrbit: string;
    minFieldOfView: string;
    maxFieldOfView: string;
    interactionPrompt: InteractionPromptStrategy;
    interactionPromptStyle: InteractionPromptStyle;
    interactionPromptThreshold: number;
    orbitSensitivity: number;
    touchAction: TouchAction;
    interpolationDecay: number;
    disableZoom: boolean;
    disablePan: boolean;
    disableTap: boolean;
    getCameraOrbit(): SphericalPosition;
    getCameraTarget(): Vector3D;
    getFieldOfView(): number;
    getMinimumFieldOfView(): number;
    getMaximumFieldOfView(): number;
    getIdealAspect(): number;
    jumpCameraToGoal(): void;
    updateFraming(): Promise<void>;
    resetInteractionPrompt(): void;
    zoom(keyPresses: number): void;
    interact(duration: number, finger0: Finger, finger1?: Finger): void;
    inputSensitivity: number;
}
export declare const ControlsMixin: <T extends Constructor<ModelViewerElementBase, object>>(ModelViewerElement: T) => {
    new (...args: any[]): ControlsInterface;
    prototype: ControlsInterface;
} & object & T;
