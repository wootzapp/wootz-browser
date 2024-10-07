import { ReactiveElement } from 'lit';
import { Vector2, Vector3 } from 'three';
import { ModelScene } from './three-components/ModelScene.js';
import { ContextLostEvent, Renderer } from './three-components/Renderer.js';
import { ProgressTracker } from './utilities/progress-tracker.js';
export declare const blobCanvas: HTMLCanvasElement;
declare const $fallbackResizeHandler: unique symbol;
declare const $defaultAriaLabel: unique symbol;
declare const $resizeObserver: unique symbol;
declare const $clearModelTimeout: unique symbol;
declare const $onContextLost: unique symbol;
declare const $loaded: unique symbol;
declare const $status: unique symbol;
declare const $onFocus: unique symbol;
declare const $onBlur: unique symbol;
export declare const $updateSize: unique symbol;
export declare const $intersectionObserver: unique symbol;
export declare const $isElementInViewport: unique symbol;
export declare const $announceModelVisibility: unique symbol;
export declare const $ariaLabel: unique symbol;
export declare const $altDefaulted: unique symbol;
export declare const $statusElement: unique symbol;
export declare const $updateStatus: unique symbol;
export declare const $loadedTime: unique symbol;
export declare const $updateSource: unique symbol;
export declare const $markLoaded: unique symbol;
export declare const $container: unique symbol;
export declare const $userInputElement: unique symbol;
export declare const $canvas: unique symbol;
export declare const $scene: unique symbol;
export declare const $needsRender: unique symbol;
export declare const $tick: unique symbol;
export declare const $onModelLoad: unique symbol;
export declare const $onResize: unique symbol;
export declare const $renderer: unique symbol;
export declare const $progressTracker: unique symbol;
export declare const $getLoaded: unique symbol;
export declare const $getModelIsVisible: unique symbol;
export declare const $shouldAttemptPreload: unique symbol;
export interface Vector3D {
    x: number;
    y: number;
    z: number;
    toString(): string;
}
export declare const toVector3D: (v: Vector3) => {
    x: number;
    y: number;
    z: number;
    toString(): string;
};
export interface Vector2D {
    u: number;
    v: number;
    toString(): string;
}
export declare const toVector2D: (v: Vector2) => {
    u: number;
    v: number;
    toString(): string;
};
interface ToBlobOptions {
    mimeType?: string;
    qualityArgument?: number;
    idealAspect?: boolean;
}
export interface FramingInfo {
    framedRadius: number;
    fieldOfViewAspect: number;
}
export interface Camera {
    viewMatrix: Array<number>;
    projectionMatrix: Array<number>;
}
export interface RendererInterface {
    load(progressCallback: (progress: number) => void): Promise<FramingInfo>;
    render(camera: Camera): void;
    resize(width: number, height: number): void;
}
/**
 * Definition for a basic <model-viewer> element.
 */
export default class ModelViewerElementBase extends ReactiveElement {
    static get is(): string;
    /** @export */
    static set modelCacheSize(value: number);
    /** @export */
    static get modelCacheSize(): number;
    /** @export */
    static set minimumRenderScale(value: number);
    /** @export */
    static get minimumRenderScale(): number;
    alt: string | null;
    src: string | null;
    withCredentials: boolean;
    /**
     * Generates a 3D model schema https://schema.org/3DModel associated with
     * the loaded src and inserts it into the header of the page for search
     * engines to crawl.
     */
    generateSchema: boolean;
    protected [$isElementInViewport]: boolean;
    protected [$loaded]: boolean;
    protected [$loadedTime]: number;
    protected [$scene]: ModelScene;
    protected [$container]: HTMLDivElement;
    protected [$userInputElement]: HTMLDivElement;
    protected [$canvas]: HTMLCanvasElement;
    protected [$statusElement]: HTMLSpanElement;
    protected [$status]: string;
    protected [$defaultAriaLabel]: string;
    protected [$clearModelTimeout]: number | null;
    protected [$fallbackResizeHandler]: (...args: any[]) => void;
    protected [$announceModelVisibility]: (...args: any[]) => void;
    protected [$resizeObserver]: ResizeObserver | null;
    protected [$intersectionObserver]: IntersectionObserver | null;
    protected [$progressTracker]: ProgressTracker;
    /** @export */
    get loaded(): boolean;
    get [$renderer](): Renderer;
    /** @export */
    get modelIsVisible(): boolean;
    /**
     * Creates a new ModelViewerElement.
     */
    constructor();
    connectedCallback(): void;
    disconnectedCallback(): void;
    updated(changedProperties: Map<string | number | symbol, any>): void;
    /** @export */
    toDataURL(type?: string, encoderOptions?: number): string;
    /** @export */
    toBlob(options?: ToBlobOptions): Promise<Blob>;
    registerRenderer(renderer: RendererInterface): void;
    unregisterRenderer(): void;
    get [$ariaLabel](): string;
    get [$altDefaulted](): string;
    [$getLoaded](): boolean;
    [$getModelIsVisible](): boolean;
    [$shouldAttemptPreload](): boolean;
    /**
     * Called on initialization and when the resize observer fires.
     */
    [$updateSize]({ width, height }: {
        width: any;
        height: any;
    }): void;
    [$tick](_time: number, _delta: number): void;
    [$markLoaded](): void;
    [$needsRender](): void;
    [$onModelLoad](): void;
    [$updateStatus](status: string): void;
    [$onFocus]: () => void;
    [$onBlur]: () => void;
    [$onResize](e: {
        width: number;
        height: number;
    }): void;
    [$onContextLost]: (event: ContextLostEvent) => void;
    /**
     * Parses the element for an appropriate source URL and
     * sets the views to use the new model based.
     */
    [$updateSource](): Promise<void>;
}
export {};
