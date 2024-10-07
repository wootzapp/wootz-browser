import { MeshStandardMaterial } from 'three';
import { GLTF, PBRMetallicRoughness as GLTFPBRMetallicRoughness } from '../../three-components/gltf-instance/gltf-2.0.js';
import { PBRMetallicRoughness as PBRMetallicRoughnessInterface, RGBA } from './api.js';
import { TextureInfo } from './texture-info.js';
import { ThreeDOMElement } from './three-dom-element.js';
declare const $threeMaterials: unique symbol;
declare const $baseColorTexture: unique symbol;
declare const $metallicRoughnessTexture: unique symbol;
/**
 * PBR material properties facade implementation for Three.js materials
 */
export declare class PBRMetallicRoughness extends ThreeDOMElement implements PBRMetallicRoughnessInterface {
    private [$baseColorTexture];
    private [$metallicRoughnessTexture];
    private get [$threeMaterials]();
    constructor(onUpdate: () => void, gltf: GLTF, pbrMetallicRoughness: GLTFPBRMetallicRoughness, correlatedMaterials: Set<MeshStandardMaterial>);
    get baseColorFactor(): RGBA;
    get metallicFactor(): number;
    get roughnessFactor(): number;
    get baseColorTexture(): TextureInfo;
    get metallicRoughnessTexture(): TextureInfo;
    setBaseColorFactor(rgba: RGBA | string): void;
    setMetallicFactor(value: number): void;
    setRoughnessFactor(value: number): void;
}
export {};
