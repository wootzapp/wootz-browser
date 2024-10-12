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
var _a, _b, _c, _d, _e, _f;
import { Mesh } from 'three';
import { $setActive, $variantSet, Material } from './material.js';
import { $children, Node, PrimitiveNode } from './nodes/primitive-node.js';
import { $correlatedObjects, $sourceObject } from './three-dom-element.js';
export const $materials = Symbol('materials');
const $hierarchy = Symbol('hierarchy');
const $roots = Symbol('roots');
export const $primitivesList = Symbol('primitives');
export const $loadVariant = Symbol('loadVariant');
export const $correlatedSceneGraph = Symbol('correlatedSceneGraph');
export const $prepareVariantsForExport = Symbol('prepareVariantsForExport');
export const $switchVariant = Symbol('switchVariant');
export const $threeScene = Symbol('threeScene');
export const $materialsFromPoint = Symbol('materialsFromPoint');
export const $materialFromPoint = Symbol('materialFromPoint');
export const $variantData = Symbol('variantData');
export const $availableVariants = Symbol('availableVariants');
const $modelOnUpdate = Symbol('modelOnUpdate');
const $cloneMaterial = Symbol('cloneMaterial');
// Holds onto temporary scene context information needed to perform lazy loading
// of a resource.
export class LazyLoader {
    constructor(gltf, gltfElementMap, mapKey, doLazyLoad) {
        this.gltf = gltf;
        this.gltfElementMap = gltfElementMap;
        this.mapKey = mapKey;
        this.doLazyLoad = doLazyLoad;
    }
}
/**
 * A Model facades the top-level GLTF object returned by Three.js' GLTFLoader.
 * Currently, the model only bothers itself with the materials in the Three.js
 * scene graph.
 */
export class Model {
    constructor(correlatedSceneGraph, onUpdate = () => { }) {
        this[_a] = new Array();
        this[_b] = new Array();
        this[_c] = new Array();
        this[_d] = new Array();
        this[_e] = () => { };
        this[_f] = new Map();
        this[$modelOnUpdate] = onUpdate;
        this[$correlatedSceneGraph] = correlatedSceneGraph;
        const { gltf, threeGLTF, gltfElementMap } = correlatedSceneGraph;
        this[$threeScene] = threeGLTF.scene;
        for (const [i, material] of gltf.materials.entries()) {
            const correlatedMaterial = gltfElementMap.get(material);
            if (correlatedMaterial != null) {
                this[$materials].push(new Material(onUpdate, gltf, material, i, true, this[$variantData], correlatedMaterial));
            }
            else {
                const elementArray = gltf['materials'] || [];
                const gltfMaterialDef = elementArray[i];
                // Loads the three.js material.
                const capturedMatIndex = i;
                const materialLoadCallback = async () => {
                    const threeMaterial = await threeGLTF.parser.getDependency('material', capturedMatIndex);
                    // Adds correlation, maps the variant gltf-def to the
                    // three material set containing the variant material.
                    const threeMaterialSet = new Set();
                    gltfElementMap.set(gltfMaterialDef, threeMaterialSet);
                    threeMaterialSet.add(threeMaterial);
                    return { set: threeMaterialSet, material: threeMaterial };
                };
                // Configures the material for lazy loading.
                this[$materials].push(new Material(onUpdate, gltf, gltfMaterialDef, i, false, this[$variantData], correlatedMaterial, new LazyLoader(gltf, gltfElementMap, gltfMaterialDef, materialLoadCallback)));
            }
        }
        // Creates a hierarchy of Nodes. Allows not just for switching which
        // material is applied to a mesh but also exposes a way to provide API
        // for switching materials and general assignment/modification.
        // Prepares for scene iteration.
        const parentMap = new Map();
        const nodeStack = new Array();
        for (const object of threeGLTF.scene.children) {
            nodeStack.push(object);
        }
        // Walks the hierarchy and creates a node tree.
        while (nodeStack.length > 0) {
            const object = nodeStack.pop();
            let node = null;
            if (object instanceof Mesh) {
                node = new PrimitiveNode(object, this.materials, this[$variantData], correlatedSceneGraph);
                this[$primitivesList].push(node);
            }
            else {
                node = new Node(object.name);
            }
            const parent = parentMap.get(object);
            if (parent != null) {
                parent[$children].push(node);
            }
            else {
                this[$roots].push(node);
            }
            this[$hierarchy].push(node);
            for (const child of object.children) {
                nodeStack.push(child);
                parentMap.set(object, node);
            }
        }
    }
    /**
     * Materials are listed in the order of the GLTF materials array, plus a
     * default material at the end if one is used.
     *
     * TODO(#1003): How do we handle non-active scenes?
     */
    get materials() {
        return this[$materials];
    }
    [(_a = $materials, _b = $hierarchy, _c = $roots, _d = $primitivesList, _e = $modelOnUpdate, _f = $variantData, $availableVariants)]() {
        const variants = Array.from(this[$variantData].values());
        variants.sort((a, b) => {
            return a.index - b.index;
        });
        return variants.map((data) => {
            return data.name;
        });
    }
    getMaterialByName(name) {
        const matches = this[$materials].filter(material => {
            return material.name === name;
        });
        if (matches.length > 0) {
            return matches[0];
        }
        return null;
    }
    /**
     * Intersects a ray with the Model and returns a list of materials whose
     * objects were intersected.
     */
    [$materialsFromPoint](raycaster) {
        const hits = raycaster.intersectObject(this[$threeScene], true);
        // Map the object hits to primitives and then to the active material of
        // the primitive.
        return hits.map((hit) => {
            const found = this[$hierarchy].find((node) => {
                if (node instanceof PrimitiveNode) {
                    const primitive = node;
                    if (primitive.mesh === hit.object) {
                        return true;
                    }
                }
                return false;
            });
            if (found != null) {
                return found.getActiveMaterial();
            }
            return null;
        });
    }
    /**
     * Intersects a ray with the Model and returns the first material whose
     * object was intersected.
     */
    [$materialFromPoint](raycaster) {
        const materials = this[$materialsFromPoint](raycaster);
        if (materials.length > 0) {
            return materials[0];
        }
        return null;
    }
    /**
     * Switches model variant to the variant name provided, or switches to
     * default/initial materials if 'null' is provided.
     */
    async [$switchVariant](variantName) {
        for (const primitive of this[$primitivesList]) {
            await primitive.enableVariant(variantName);
        }
        for (const material of this.materials) {
            material[$setActive](false);
        }
        // Marks the materials that are now in use after the variant switch.
        for (const primitive of this[$primitivesList]) {
            this.materials[primitive.getActiveMaterial().index][$setActive](true);
        }
    }
    async [$prepareVariantsForExport]() {
        const promises = new Array();
        for (const primitive of this[$primitivesList]) {
            promises.push(primitive.instantiateVariants());
        }
        await Promise.all(promises);
    }
    [$cloneMaterial](index, newMaterialName) {
        const material = this.materials[index];
        if (!material.isLoaded) {
            console.error(`Cloning an unloaded material,
           call 'material.ensureLoaded() before cloning the material.`);
        }
        const threeMaterialSet = material[$correlatedObjects];
        // clones the gltf material data and updates the material name.
        const gltfSourceMaterial = JSON.parse(JSON.stringify(material[$sourceObject]));
        gltfSourceMaterial.name = newMaterialName;
        // Adds the source material clone to the gltf def.
        const gltf = this[$correlatedSceneGraph].gltf;
        gltf.materials.push(gltfSourceMaterial);
        const clonedSet = new Set();
        for (const [i, threeMaterial] of threeMaterialSet.entries()) {
            const clone = threeMaterial.clone();
            clone.name =
                newMaterialName + (threeMaterialSet.size > 1 ? '_inst' + i : '');
            clonedSet.add(clone);
        }
        const clonedMaterial = new Material(this[$modelOnUpdate], this[$correlatedSceneGraph].gltf, gltfSourceMaterial, this[$materials].length, false, // Cloned as inactive.
        this[$variantData], clonedSet);
        this[$materials].push(clonedMaterial);
        return clonedMaterial;
    }
    createMaterialInstanceForVariant(originalMaterialIndex, newMaterialName, variantName, activateVariant = true) {
        let variantMaterialInstance = null;
        for (const primitive of this[$primitivesList]) {
            const variantData = this[$variantData].get(variantName);
            // Skips the primitive if the variant already exists.
            if (variantData != null && primitive.variantInfo.has(variantData.index)) {
                continue;
            }
            // Skips the primitive if the source/original material does not exist.
            if (primitive.getMaterial(originalMaterialIndex) == null) {
                continue;
            }
            if (!this.hasVariant(variantName)) {
                this.createVariant(variantName);
            }
            if (variantMaterialInstance == null) {
                variantMaterialInstance =
                    this[$cloneMaterial](originalMaterialIndex, newMaterialName);
            }
            primitive.addVariant(variantMaterialInstance, variantName);
        }
        if (activateVariant && variantMaterialInstance != null) {
            variantMaterialInstance[$setActive](true);
            this.materials[originalMaterialIndex][$setActive](false);
            for (const primitive of this[$primitivesList]) {
                primitive.enableVariant(variantName);
            }
        }
        return variantMaterialInstance;
    }
    createVariant(variantName) {
        if (!this[$variantData].has(variantName)) {
            // Adds the name if it's not already in the list.
            this[$variantData].set(variantName, { name: variantName, index: this[$variantData].size });
        }
        else {
            console.warn(`Variant '${variantName}'' already exists`);
        }
    }
    hasVariant(variantName) {
        return this[$variantData].has(variantName);
    }
    setMaterialToVariant(materialIndex, targetVariantName) {
        if (this[$availableVariants]().find(name => name === targetVariantName) ==
            null) {
            console.warn(`Can't add material to '${targetVariantName}', the variant does not exist.'`);
            return;
        }
        if (materialIndex < 0 || materialIndex >= this.materials.length) {
            console.error(`setMaterialToVariant(): materialIndex is out of bounds.`);
            return;
        }
        for (const primitive of this[$primitivesList]) {
            const material = primitive.getMaterial(materialIndex);
            // Ensures the material exists on the primitive before setting it to a
            // variant.
            if (material != null) {
                primitive.addVariant(material, targetVariantName);
            }
        }
    }
    updateVariantName(currentName, newName) {
        const variantData = this[$variantData].get(currentName);
        if (variantData == null) {
            return;
        }
        variantData.name = newName;
        this[$variantData].set(newName, variantData);
        this[$variantData].delete(currentName);
    }
    deleteVariant(variantName) {
        const variant = this[$variantData].get(variantName);
        if (variant == null) {
            return;
        }
        for (const material of this.materials) {
            if (material.hasVariant(variantName)) {
                material[$variantSet].delete(variant.index);
            }
        }
        for (const primitive of this[$primitivesList]) {
            primitive.deleteVariant(variant.index);
        }
        this[$variantData].delete(variantName);
    }
}
//# sourceMappingURL=model.js.map