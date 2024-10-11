import { Texture as ThreeTexture } from 'three';
import { MagFilter, MinFilter, Sampler as GLTFSampler, WrapMode } from '../../three-components/gltf-instance/gltf-2.0.js';
import { Sampler as SamplerInterface } from './api.js';
import { ThreeDOMElement } from './three-dom-element.js';
declare const $threeTextures: unique symbol;
declare const $setProperty: unique symbol;
declare const $sourceSampler: unique symbol;
/**
 * Sampler facade implementation for Three.js textures
 */
export declare class Sampler extends ThreeDOMElement implements SamplerInterface {
    private get [$threeTextures]();
    private get [$sourceSampler]();
    constructor(onUpdate: () => void, texture: ThreeTexture | null, gltfSampler: GLTFSampler | null);
    get name(): string;
    get minFilter(): MinFilter;
    get magFilter(): MagFilter;
    get wrapS(): WrapMode;
    get wrapT(): WrapMode;
    setMinFilter(filter: MinFilter): void;
    setMagFilter(filter: MagFilter): void;
    setWrapS(mode: WrapMode): void;
    setWrapT(mode: WrapMode): void;
    private [$setProperty];
}
export {};
