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
import { Color } from 'three';
import { TextureInfo, TextureUsage } from './texture-info.js';
import { $correlatedObjects, $onUpdate, $sourceObject, ThreeDOMElement } from './three-dom-element.js';
const $threeMaterials = Symbol('threeMaterials');
const $baseColorTexture = Symbol('baseColorTexture');
const $metallicRoughnessTexture = Symbol('metallicRoughnessTexture');
/**
 * PBR material properties facade implementation for Three.js materials
 */
export class PBRMetallicRoughness extends ThreeDOMElement {
    constructor(onUpdate, gltf, pbrMetallicRoughness, correlatedMaterials) {
        super(onUpdate, pbrMetallicRoughness, correlatedMaterials);
        // Assign glTF default values
        if (pbrMetallicRoughness.baseColorFactor == null) {
            pbrMetallicRoughness.baseColorFactor = [1, 1, 1, 1];
        }
        if (pbrMetallicRoughness.roughnessFactor == null) {
            pbrMetallicRoughness.roughnessFactor = 1;
        }
        if (pbrMetallicRoughness.metallicFactor == null) {
            pbrMetallicRoughness.metallicFactor = 1;
        }
        const { baseColorTexture: gltfBaseColorTexture, metallicRoughnessTexture: gltfMetallicRoughnessTexture } = pbrMetallicRoughness;
        const { map, metalnessMap } = correlatedMaterials.values().next().value;
        this[$baseColorTexture] = new TextureInfo(onUpdate, TextureUsage.Base, map, correlatedMaterials, gltf, gltfBaseColorTexture ? gltfBaseColorTexture : null);
        this[$metallicRoughnessTexture] = new TextureInfo(onUpdate, TextureUsage.MetallicRoughness, metalnessMap, correlatedMaterials, gltf, gltfMetallicRoughnessTexture ? gltfMetallicRoughnessTexture : null);
    }
    get [$threeMaterials]() {
        return this[$correlatedObjects];
    }
    get baseColorFactor() {
        return this[$sourceObject]
            .baseColorFactor;
    }
    get metallicFactor() {
        return this[$sourceObject]
            .metallicFactor;
    }
    get roughnessFactor() {
        return this[$sourceObject]
            .roughnessFactor;
    }
    get baseColorTexture() {
        return this[$baseColorTexture];
    }
    get metallicRoughnessTexture() {
        return this[$metallicRoughnessTexture];
    }
    setBaseColorFactor(rgba) {
        const color = new Color();
        if (rgba instanceof Array) {
            color.fromArray(rgba);
        }
        else {
            color.set(rgba).convertSRGBToLinear();
        }
        for (const material of this[$threeMaterials]) {
            material.color.set(color);
            if (rgba instanceof Array) {
                material.opacity = (rgba)[3];
            }
            else {
                rgba = [0, 0, 0, material.opacity];
                color.toArray(rgba);
            }
        }
        const pbrMetallicRoughness = this[$sourceObject];
        pbrMetallicRoughness.baseColorFactor = rgba;
        this[$onUpdate]();
    }
    setMetallicFactor(value) {
        for (const material of this[$threeMaterials]) {
            material.metalness = value;
        }
        const pbrMetallicRoughness = this[$sourceObject];
        pbrMetallicRoughness.metallicFactor = value;
        this[$onUpdate]();
    }
    setRoughnessFactor(value) {
        for (const material of this[$threeMaterials]) {
            material.roughness = value;
        }
        const pbrMetallicRoughness = this[$sourceObject];
        pbrMetallicRoughness.roughnessFactor = value;
        this[$onUpdate]();
    }
}
//# sourceMappingURL=pbr-metallic-roughness.js.map