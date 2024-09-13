import { Material as ThreeMaterial, Raycaster } from 'three';
import { CorrelatedSceneGraph, GLTFElementToThreeObjectMap, ThreeObjectSet } from '../../three-components/gltf-instance/correlated-scene-graph.js';
import { GLTF, GLTFElement } from '../../three-components/gltf-instance/gltf-2.0.js';
import { Model as ModelInterface } from './api.js';
import { Material } from './material.js';
export declare const $materials: unique symbol;
declare const $hierarchy: unique symbol;
declare const $roots: unique symbol;
export declare const $primitivesList: unique symbol;
export declare const $loadVariant: unique symbol;
export declare const $correlatedSceneGraph: unique symbol;
export declare const $prepareVariantsForExport: unique symbol;
export declare const $switchVariant: unique symbol;
export declare const $threeScene: unique symbol;
export declare const $materialsFromPoint: unique symbol;
export declare const $materialFromPoint: unique symbol;
export declare const $variantData: unique symbol;
export declare const $availableVariants: unique symbol;
declare const $modelOnUpdate: unique symbol;
declare const $cloneMaterial: unique symbol;
export declare class LazyLoader {
    gltf: GLTF;
    gltfElementMap: GLTFElementToThreeObjectMap;
    mapKey: GLTFElement;
    doLazyLoad: () => Promise<{
        set: ThreeObjectSet;
        material: ThreeMaterial;
    }>;
    constructor(gltf: GLTF, gltfElementMap: GLTFElementToThreeObjectMap, mapKey: GLTFElement, doLazyLoad: () => Promise<{
        set: ThreeObjectSet;
        material: ThreeMaterial;
    }>);
}
/**
 * Facades variant mapping data.
 */
export interface VariantData {
    name: string;
    index: number;
}
/**
 * A Model facades the top-level GLTF object returned by Three.js' GLTFLoader.
 * Currently, the model only bothers itself with the materials in the Three.js
 * scene graph.
 */
export declare class Model implements ModelInterface {
    private [$materials];
    private [$hierarchy];
    private [$roots];
    private [$primitivesList];
    private [$threeScene];
    private [$modelOnUpdate];
    private [$correlatedSceneGraph];
    private [$variantData];
    constructor(correlatedSceneGraph: CorrelatedSceneGraph, onUpdate?: () => void);
    /**
     * Materials are listed in the order of the GLTF materials array, plus a
     * default material at the end if one is used.
     *
     * TODO(#1003): How do we handle non-active scenes?
     */
    get materials(): Material[];
    [$availableVariants](): string[];
    getMaterialByName(name: string): Material | null;
    /**
     * Intersects a ray with the Model and returns a list of materials whose
     * objects were intersected.
     */
    [$materialsFromPoint](raycaster: Raycaster): Material[];
    /**
     * Intersects a ray with the Model and returns the first material whose
     * object was intersected.
     */
    [$materialFromPoint](raycaster: Raycaster): Material | null;
    /**
     * Switches model variant to the variant name provided, or switches to
     * default/initial materials if 'null' is provided.
     */
    [$switchVariant](variantName: string | null): Promise<void>;
    [$prepareVariantsForExport](): Promise<void>;
    [$cloneMaterial](index: number, newMaterialName: string): Material;
    createMaterialInstanceForVariant(originalMaterialIndex: number, newMaterialName: string, variantName: string, activateVariant?: boolean): Material | null;
    createVariant(variantName: string): void;
    hasVariant(variantName: string): boolean;
    setMaterialToVariant(materialIndex: number, targetVariantName: string): void;
    updateVariantName(currentName: string, newName: string): void;
    deleteVariant(variantName: string): void;
}
export {};
