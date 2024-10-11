/// <reference types="webxr" />
import { Event, EventDispatcher, WebGLRenderer } from 'three';
import ModelViewerElementBase from '../model-viewer-base.js';
import { ARRenderer } from './ARRenderer.js';
import { CachingGLTFLoader } from './CachingGLTFLoader.js';
import { Debugger } from './Debugger.js';
import { ModelViewerGLTFInstance } from './gltf-instance/ModelViewerGLTFInstance.js';
import { ModelScene } from './ModelScene.js';
import TextureUtils from './TextureUtils.js';
export interface RendererOptions {
    powerPreference: string;
    debug?: boolean;
}
export interface ContextLostEvent extends Event {
    type: 'contextlost';
    sourceEvent: WebGLContextEvent;
}
export declare const DEFAULT_POWER_PREFERENCE: string;
/**
 * Registers canvases with Canvas2DRenderingContexts and renders them
 * all in the same WebGLRenderingContext, spitting out textures to apply
 * to the canvases. Creates a fullscreen WebGL canvas that is not added
 * to the DOM, and on each frame, renders each registered canvas on a portion
 * of the WebGL canvas, and applies the texture on the registered canvas.
 *
 * In the future, can use ImageBitmapRenderingContext instead of
 * Canvas2DRenderingContext if supported for cheaper transferring of
 * the texture.
 */
export declare class Renderer extends EventDispatcher {
    private static _singleton;
    static get singleton(): Renderer;
    static resetSingleton(): void;
    threeRenderer: WebGLRenderer;
    canvas3D: HTMLCanvasElement;
    textureUtils: TextureUtils | null;
    arRenderer: ARRenderer;
    loader: CachingGLTFLoader<typeof ModelViewerGLTFInstance>;
    width: number;
    height: number;
    dpr: number;
    protected debugger: Debugger | null;
    private scenes;
    private multipleScenesVisible;
    private lastTick;
    private renderedLastFrame;
    private scaleStep;
    private lastStep;
    private avgFrameDuration;
    get canRender(): boolean;
    get scaleFactor(): number;
    set minScale(scale: number);
    constructor(options: RendererOptions);
    registerScene(scene: ModelScene): void;
    unregisterScene(scene: ModelScene): void;
    displayCanvas(scene: ModelScene): HTMLCanvasElement;
    /**
     * The function enables an optimization, where when there is only a single
     * <model-viewer> element, we can use the renderer's 3D canvas directly for
     * display. Otherwise we need to use the element's 2D canvas and copy the
     * renderer's result into it.
     */
    private countVisibleScenes;
    /**
     * Updates the renderer's size based on the largest scene and any changes to
     * device pixel ratio.
     */
    private updateRendererSize;
    private updateRendererScale;
    private shouldRender;
    private rescaleCanvas;
    private sceneSize;
    private copyPixels;
    /**
     * Returns an array version of this.scenes where the non-visible ones are
     * first. This allows eager scenes to be rendered before they are visible,
     * without needing the multi-canvas render path.
     */
    private orderedScenes;
    get isPresenting(): boolean;
    /**
     * This method takes care of updating the element and renderer state based on
     * the time that has passed since the last rendered frame.
     */
    preRender(scene: ModelScene, t: number, delta: number): void;
    render(t: number, frame?: XRFrame): void;
    dispose(): Array<ModelViewerElementBase>;
    onWebGLContextLost: (event: Event) => void;
    onWebGLContextRestored: () => void;
}
