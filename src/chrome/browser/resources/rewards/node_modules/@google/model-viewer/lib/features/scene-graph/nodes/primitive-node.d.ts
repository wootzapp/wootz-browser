import { Material as ThreeMaterial, Mesh } from 'three';
import { CorrelatedSceneGraph } from '../../../three-components/gltf-instance/correlated-scene-graph.js';
import { Material } from '../material.js';
import { VariantData } from '../model.js';
declare const $materials: unique symbol;
declare const $variantToMaterialMap: unique symbol;
declare const $modelVariants: unique symbol;
declare const $mesh: unique symbol;
export declare const $primitives: unique symbol;
export declare const $loadVariant: unique symbol;
export declare const $prepareVariantsForExport: unique symbol;
export declare const $switchVariant: unique symbol;
export declare const $children: unique symbol;
export declare const $initialMaterialIdx: unique symbol;
export declare const $activeMaterialIdx: unique symbol;
export declare class Node {
    name: string;
    [$children]: Node[];
    constructor(name: string);
}
export declare class PrimitiveNode extends Node {
    private [$mesh];
    [$materials]: Map<number, Material>;
    private [$variantToMaterialMap];
    private [$initialMaterialIdx];
    private [$activeMaterialIdx];
    private [$modelVariants];
    constructor(mesh: Mesh, mvMaterials: Material[], modelVariants: Map<string, VariantData>, correlatedSceneGraph: CorrelatedSceneGraph);
    get mesh(): Mesh<import("three").BufferGeometry, ThreeMaterial | ThreeMaterial[]>;
    setActiveMaterial(material: number): Promise<ThreeMaterial | null>;
    getActiveMaterial(): Material;
    getMaterial(index: number): Material | undefined;
    enableVariant(name: string | null): Promise<ThreeMaterial | null>;
    private enableVariantHelper;
    instantiateVariants(): Promise<void>;
    get variantInfo(): Map<number, Material>;
    addVariant(materialVariant: Material, variantName: string): boolean;
    deleteVariant(variantIndex: number): void;
    private updateVariantUserData;
    private ensureVariantIsUnused;
}
export {};
