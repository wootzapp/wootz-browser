import { MeshStandardMaterial, Texture as ThreeTexture } from 'three';
import { GLTF, TextureInfo as GLTFTextureInfo } from '../../three-components/gltf-instance/gltf-2.0.js';
import { TextureInfo as TextureInfoInterface } from './api.js';
import { Texture } from './texture.js';
declare const $texture: unique symbol;
declare const $transform: unique symbol;
export declare const $materials: unique symbol;
export declare const $usage: unique symbol;
export declare enum TextureUsage {
    Base = 0,
    MetallicRoughness = 1,
    Normal = 2,
    Occlusion = 3,
    Emissive = 4
}
/**
 * TextureInfo facade implementation for Three.js materials
 */
export declare class TextureInfo implements TextureInfoInterface {
    private [$texture];
    private [$transform];
    [$materials]: Set<MeshStandardMaterial> | null;
    [$usage]: TextureUsage;
    onUpdate: () => void;
    constructor(onUpdate: () => void, usage: TextureUsage, threeTexture: ThreeTexture | null, material: Set<MeshStandardMaterial>, gltf: GLTF, gltfTextureInfo: GLTFTextureInfo | null);
    get texture(): Texture | null;
    setTexture(texture: Texture | null): void;
}
export {};
