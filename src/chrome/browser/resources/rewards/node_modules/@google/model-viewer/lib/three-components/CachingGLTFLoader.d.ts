import { Event as ThreeEvent, EventDispatcher, WebGLRenderer } from 'three';
import { GLTF, GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader.js';
import ModelViewerElementBase from '../model-viewer-base.js';
import { CacheEvictionPolicy } from '../utilities/cache-eviction-policy.js';
import { GLTFInstance, GLTFInstanceConstructor } from './GLTFInstance.js';
export declare type ProgressCallback = (progress: number) => void;
export interface PreloadEvent extends ThreeEvent {
    type: 'preload';
    element: ModelViewerElementBase;
    src: String;
}
/**
 * A helper to Promise-ify a Three.js GLTFLoader
 */
export declare const loadWithLoader: (url: string, loader: GLTFLoader, progressCallback?: ProgressCallback) => Promise<GLTF>;
interface MeshoptDecoder {
    ready: Promise<void>;
    supported: boolean;
}
declare global {
    const MeshoptDecoder: MeshoptDecoder;
}
export declare const $loader: unique symbol;
export declare const $evictionPolicy: unique symbol;
declare const $GLTFInstance: unique symbol;
export declare class CachingGLTFLoader<T extends GLTFInstanceConstructor = GLTFInstanceConstructor> extends EventDispatcher {
    static withCredentials: boolean;
    static setDRACODecoderLocation(url: string): void;
    static getDRACODecoderLocation(): string;
    static setKTX2TranscoderLocation(url: string): void;
    static getKTX2TranscoderLocation(): string;
    static setMeshoptDecoderLocation(url: string): void;
    static getMeshoptDecoderLocation(): string;
    static initializeKTX2Loader(renderer: WebGLRenderer): void;
    static [$evictionPolicy]: CacheEvictionPolicy;
    static get cache(): Map<string, Promise<GLTFInstance>>;
    /** @nocollapse */
    static clearCache(): void;
    static has(url: string): boolean;
    /** @nocollapse */
    static delete(url: string): Promise<void>;
    /**
     * Returns true if the model that corresponds to the specified url is
     * available in our local cache.
     */
    static hasFinishedLoading(url: string): boolean;
    constructor(GLTFInstance: T);
    protected [$loader]: GLTFLoader;
    protected [$GLTFInstance]: T;
    protected get [$evictionPolicy](): CacheEvictionPolicy;
    /**
     * Preloads a glTF, populating the cache. Returns a promise that resolves
     * when the cache is populated.
     */
    preload(url: string, element: ModelViewerElementBase, progressCallback?: ProgressCallback): Promise<void>;
    /**
     * Loads a glTF from the specified url and resolves a unique clone of the
     * glTF. If the glTF has already been loaded, makes a clone of the cached
     * copy.
     */
    load(url: string, element: ModelViewerElementBase, progressCallback?: ProgressCallback): Promise<InstanceType<T>>;
}
export {};
