/* @license
 * Copyright 2020 Google LLC. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
var _a;
import { Color, DoubleSide, FrontSide } from 'three';
import { PBRMetallicRoughness } from './pbr-metallic-roughness.js';
import { TextureInfo, TextureUsage } from './texture-info.js';
import { $correlatedObjects, $onUpdate, $sourceObject, ThreeDOMElement } from './three-dom-element.js';
const $pbrMetallicRoughness = Symbol('pbrMetallicRoughness');
const $normalTexture = Symbol('normalTexture');
const $occlusionTexture = Symbol('occlusionTexture');
const $emissiveTexture = Symbol('emissiveTexture');
const $backingThreeMaterial = Symbol('backingThreeMaterial');
const $applyAlphaCutoff = Symbol('applyAlphaCutoff');
export const $lazyLoadGLTFInfo = Symbol('lazyLoadGLTFInfo');
const $initialize = Symbol('initialize');
export const $getLoadedMaterial = Symbol('getLoadedMaterial');
export const $ensureMaterialIsLoaded = Symbol('ensureMaterialIsLoaded');
export const $gltfIndex = Symbol('gltfIndex');
export const $setActive = Symbol('setActive');
export const $variantIndices = Symbol('variantIndices');
const $isActive = Symbol('isActive');
export const $variantSet = Symbol('variantSet');
const $modelVariants = Symbol('modelVariants');
/**
 * Material facade implementation for Three.js materials
 */
export class Material extends ThreeDOMElement {
    constructor(onUpdate, gltf, gltfMaterial, gltfIndex, isActive, modelVariants, correlatedMaterials, lazyLoadInfo = undefined) {
        super(onUpdate, gltfMaterial, correlatedMaterials);
        this[_a] = new Set();
        this[$gltfIndex] = gltfIndex;
        this[$isActive] = isActive;
        this[$modelVariants] = modelVariants;
        if (lazyLoadInfo == null) {
            this[$initialize](gltf);
        }
        else {
            this[$lazyLoadGLTFInfo] = lazyLoadInfo;
        }
    }
    get [(_a = $variantSet, $backingThreeMaterial)]() {
        return this[$correlatedObjects]
            .values()
            .next()
            .value;
    }
    [$initialize](gltf) {
        const onUpdate = this[$onUpdate];
        const gltfMaterial = this[$sourceObject];
        const correlatedMaterials = this[$correlatedObjects];
        if (gltfMaterial.extensions &&
            gltfMaterial.extensions['KHR_materials_pbrSpecularGlossiness']) {
            console.warn(`Material ${gltfMaterial.name} uses a deprecated extension
          "KHR_materials_pbrSpecularGlossiness", please use
          "pbrMetallicRoughness" instead. Specular Glossiness materials are
          currently supported for rendering, but not for our scene-graph API,
          nor for auto-generation of USDZ for Quick Look.`);
        }
        if (gltfMaterial.pbrMetallicRoughness == null) {
            gltfMaterial.pbrMetallicRoughness = {};
        }
        this[$pbrMetallicRoughness] = new PBRMetallicRoughness(onUpdate, gltf, gltfMaterial.pbrMetallicRoughness, correlatedMaterials);
        if (gltfMaterial.emissiveFactor == null) {
            gltfMaterial.emissiveFactor = [0, 0, 0];
        }
        if (gltfMaterial.doubleSided == null) {
            gltfMaterial.doubleSided = false;
        }
        if (gltfMaterial.alphaMode == null) {
            gltfMaterial.alphaMode = 'OPAQUE';
        }
        if (gltfMaterial.alphaCutoff == null) {
            gltfMaterial.alphaCutoff = 0.5;
        }
        const { normalTexture: gltfNormalTexture, occlusionTexture: gltfOcclusionTexture, emissiveTexture: gltfEmissiveTexture } = gltfMaterial;
        const { normalMap, aoMap, emissiveMap } = correlatedMaterials.values().next().value;
        this[$normalTexture] = new TextureInfo(onUpdate, TextureUsage.Normal, normalMap, correlatedMaterials, gltf, gltfNormalTexture ? gltfNormalTexture : null);
        this[$occlusionTexture] = new TextureInfo(onUpdate, TextureUsage.Occlusion, aoMap, correlatedMaterials, gltf, gltfOcclusionTexture ? gltfOcclusionTexture : null);
        this[$emissiveTexture] = new TextureInfo(onUpdate, TextureUsage.Emissive, emissiveMap, correlatedMaterials, gltf, gltfEmissiveTexture ? gltfEmissiveTexture : null);
    }
    async [$getLoadedMaterial]() {
        if (this[$lazyLoadGLTFInfo] != null) {
            const { set, material } = await this[$lazyLoadGLTFInfo].doLazyLoad();
            // Fills in the missing data.
            this[$correlatedObjects] = set;
            this[$initialize](this[$lazyLoadGLTFInfo].gltf);
            // Releases lazy load info.
            this[$lazyLoadGLTFInfo] = undefined;
            // Redefines the method as a noop method.
            this.ensureLoaded = async () => { };
            return material;
        }
        return this[$correlatedObjects].values().next().value;
    }
    [$ensureMaterialIsLoaded]() {
        if (this[$lazyLoadGLTFInfo] == null) {
            return;
        }
        throw new Error(`Material "${this.name}" has not been loaded, call 'await
    myMaterial.ensureLoaded()' before using an unloaded material.`);
    }
    async ensureLoaded() {
        await this[$getLoadedMaterial]();
    }
    get isLoaded() {
        return this[$lazyLoadGLTFInfo] == null;
    }
    get isActive() {
        return this[$isActive];
    }
    [$setActive](isActive) {
        this[$isActive] = isActive;
    }
    get name() {
        return this[$sourceObject].name;
    }
    set name(name) {
        const sourceMaterial = this[$sourceObject];
        if (sourceMaterial != null) {
            sourceMaterial.name = name;
        }
        if (this[$correlatedObjects] != null) {
            for (const threeMaterial of this[$correlatedObjects]) {
                threeMaterial.name = name;
            }
        }
    }
    get pbrMetallicRoughness() {
        this[$ensureMaterialIsLoaded]();
        return this[$pbrMetallicRoughness];
    }
    get normalTexture() {
        this[$ensureMaterialIsLoaded]();
        return this[$normalTexture];
    }
    get occlusionTexture() {
        this[$ensureMaterialIsLoaded]();
        return this[$occlusionTexture];
    }
    get emissiveTexture() {
        this[$ensureMaterialIsLoaded]();
        return this[$emissiveTexture];
    }
    get emissiveFactor() {
        this[$ensureMaterialIsLoaded]();
        return this[$sourceObject].emissiveFactor;
    }
    get index() {
        return this[$gltfIndex];
    }
    [$variantIndices]() {
        return this[$variantSet];
    }
    hasVariant(name) {
        const variantData = this[$modelVariants].get(name);
        return variantData != null && this[$variantSet].has(variantData.index);
    }
    setEmissiveFactor(rgb) {
        this[$ensureMaterialIsLoaded]();
        const color = new Color();
        if (rgb instanceof Array) {
            color.fromArray(rgb);
        }
        else {
            color.set(rgb).convertSRGBToLinear();
        }
        for (const material of this[$correlatedObjects]) {
            material.emissive.set(color);
        }
        this[$sourceObject].emissiveFactor =
            color.toArray();
        this[$onUpdate]();
    }
    [$applyAlphaCutoff]() {
        this[$ensureMaterialIsLoaded]();
        const gltfMaterial = this[$sourceObject];
        for (const material of this[$correlatedObjects]) {
            if (this[$sourceObject].alphaMode === 'MASK') {
                material.alphaTest = gltfMaterial.alphaCutoff;
            }
            else {
                material.alphaTest = undefined;
            }
            material.needsUpdate = true;
        }
    }
    setAlphaCutoff(cutoff) {
        this[$ensureMaterialIsLoaded]();
        this[$sourceObject].alphaCutoff = cutoff;
        this[$applyAlphaCutoff]();
        this[$onUpdate]();
    }
    getAlphaCutoff() {
        this[$ensureMaterialIsLoaded]();
        return this[$sourceObject].alphaCutoff;
    }
    setDoubleSided(doubleSided) {
        this[$ensureMaterialIsLoaded]();
        for (const material of this[$correlatedObjects]) {
            // When double-sided is disabled gltf spec dictates that Back-Face culling
            // must be disabled, in three.js parlance that would mean FrontSide
            // rendering only.
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#double-sided
            material.side = doubleSided ? DoubleSide : FrontSide;
            material.needsUpdate = true;
        }
        this[$sourceObject].doubleSided = doubleSided;
        this[$onUpdate]();
    }
    getDoubleSided() {
        this[$ensureMaterialIsLoaded]();
        return this[$sourceObject].doubleSided;
    }
    setAlphaMode(alphaMode) {
        this[$ensureMaterialIsLoaded]();
        const enableTransparency = (material, enabled) => {
            material.transparent = enabled;
            material.depthWrite = !enabled;
        };
        this[$sourceObject].alphaMode = alphaMode;
        for (const material of this[$correlatedObjects]) {
            enableTransparency(material, alphaMode === 'BLEND');
            this[$applyAlphaCutoff]();
            material.needsUpdate = true;
        }
        this[$onUpdate]();
    }
    getAlphaMode() {
        this[$ensureMaterialIsLoaded]();
        return this[$sourceObject].alphaMode;
    }
}
//# sourceMappingURL=material.js.map