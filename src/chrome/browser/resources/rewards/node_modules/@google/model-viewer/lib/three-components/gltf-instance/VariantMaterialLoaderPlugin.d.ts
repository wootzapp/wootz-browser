/**
 * Materials variants extension
 *
 * Specification:
 * https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_variants
 */
/**
 * The code in this file is based on
 * https://github.com/takahirox/three-gltf-extensions/tree/main/loaders/KHR_materials_variants
 */
import { Material as ThreeMaterial } from 'three';
import { GLTF, GLTFLoaderPlugin, GLTFParser } from 'three/examples/jsm/loaders/GLTFLoader.js';
export interface UserDataVariantMapping {
    material: ThreeMaterial | null;
    gltfMaterialIndex: number;
}
export default class GLTFMaterialsVariantsExtension implements GLTFLoaderPlugin {
    parser: GLTFParser;
    name: string;
    constructor(parser: GLTFParser);
    afterRoot(gltf: GLTF): Promise<void> | null;
}
