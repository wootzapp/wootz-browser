/**
 * Materials variants extension
 *
 * Specification:
 * https://github.com/takahirox/three-gltf-extensions/tree/main/loaders/KHR_materials_variants
 */
/**
 * The code in this file is based on
 * https://github.com/takahirox/three-gltf-extensions/tree/main/exporters/KHR_materials_variants
 */
import { Mesh, Object3D } from 'three';
export default class GLTFExporterMaterialsVariantsExtension {
    writer: any;
    name: string;
    variantNames: string[];
    constructor(writer: any);
    beforeParse(objects: Object3D[]): void;
    writeMesh(mesh: Mesh, meshDef: any): void;
    afterParse(): void;
}
