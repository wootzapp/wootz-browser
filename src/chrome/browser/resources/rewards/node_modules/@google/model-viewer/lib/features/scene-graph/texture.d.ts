import { Texture as ThreeTexture } from 'three';
import { Image as GLTFImage, Sampler as GLTFSampler, Texture as GLTFTexture } from '../../three-components/gltf-instance/gltf-2.0.js';
import { Texture as TextureInterface } from './api.js';
import { Image } from './image.js';
import { Sampler } from './sampler.js';
import { ThreeDOMElement } from './three-dom-element.js';
declare const $image: unique symbol;
declare const $sampler: unique symbol;
/**
 * Material facade implementation for Three.js materials
 */
export declare class Texture extends ThreeDOMElement implements TextureInterface {
    private [$image];
    private [$sampler];
    constructor(onUpdate: () => void, threeTexture: ThreeTexture | null, gltfTexture?: GLTFTexture | null, gltfSampler?: GLTFSampler | null, gltfImage?: GLTFImage | null);
    get name(): string;
    set name(name: string);
    get sampler(): Sampler;
    get source(): Image;
}
export {};
