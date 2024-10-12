import { Texture as ThreeTexture } from 'three';
import { Image as GLTFImage } from '../../three-components/gltf-instance/gltf-2.0.js';
import { Image as ImageInterface } from './api.js';
import { ThreeDOMElement } from './three-dom-element.js';
export declare const $threeTexture: unique symbol;
export declare const $applyTexture: unique symbol;
/**
 * Image facade implementation for Three.js textures
 */
export declare class Image extends ThreeDOMElement implements ImageInterface {
    get [$threeTexture](): ThreeTexture;
    constructor(onUpdate: () => void, texture: ThreeTexture | null, gltfImage: GLTFImage | null);
    get name(): string;
    get uri(): string | undefined;
    get bufferView(): number | undefined;
    get type(): 'embedded' | 'external';
    set name(name: string);
    createThumbnail(width: number, height: number): Promise<string>;
}
