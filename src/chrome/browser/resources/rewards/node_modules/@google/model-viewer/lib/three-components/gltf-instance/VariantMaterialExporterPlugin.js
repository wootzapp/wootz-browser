/* @license
 * Copyright 2021 Google LLC. All Rights Reserved.
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
/**
 * @param object {THREE.Object3D}
 * @return {boolean}
 */
const compatibleObject = (object) => {
    // @TODO: Need properer variantMaterials format validation?
    return object.material !==
        undefined && // easier than (!object.isMesh && !object.isLine &&
        // !object.isPoints)
        object.userData && // just in case
        object.userData.variantMaterials &&
        // Is this line costly?
        !!Array
            .from(object.userData.variantMaterials
            .values())
            .filter(m => compatibleMaterial(m.material));
};
/**
 * @param material {THREE.Material}
 * @return {boolean}
 */
const compatibleMaterial = (material) => {
    // @TODO: support multi materials?
    return material && material.isMaterial && !Array.isArray(material);
};
export default class GLTFExporterMaterialsVariantsExtension {
    constructor(writer) {
        this.writer = writer;
        this.name = 'KHR_materials_variants';
        this.variantNames = [];
    }
    beforeParse(objects) {
        // Find all variant names and store them to the table
        const variantNameSet = new Set();
        for (const object of objects) {
            object.traverse(o => {
                if (!compatibleObject(o)) {
                    return;
                }
                const variantMaterials = o.userData.variantMaterials;
                const variantDataMap = o.userData.variantData;
                for (const [variantName, variantData] of variantDataMap) {
                    const variantMaterial = variantMaterials.get(variantData.index);
                    // Ignore unloaded variant materials
                    if (variantMaterial && compatibleMaterial(variantMaterial.material)) {
                        variantNameSet.add(variantName);
                    }
                }
            });
        }
        // We may want to sort?
        variantNameSet.forEach(name => this.variantNames.push(name));
    }
    writeMesh(mesh, meshDef) {
        if (!compatibleObject(mesh)) {
            return;
        }
        const userData = mesh.userData;
        const variantMaterials = userData.variantMaterials;
        const variantDataMap = userData.variantData;
        const mappingTable = new Map();
        // Removes gaps in the variant indices list (caused by deleting variants).
        const reIndexedVariants = new Map();
        const variants = Array.from(variantDataMap.values()).sort((a, b) => {
            return a.index - b.index;
        });
        for (const [i, variantData] of variants.entries()) {
            reIndexedVariants.set(variantData.index, i);
        }
        for (const variantData of variantDataMap.values()) {
            const variantInstance = variantMaterials.get(variantData.index);
            if (!variantInstance || !compatibleMaterial(variantInstance.material)) {
                continue;
            }
            const materialIndex = this.writer.processMaterial(variantInstance.material);
            if (!mappingTable.has(materialIndex)) {
                mappingTable.set(materialIndex, { material: materialIndex, variants: [] });
            }
            mappingTable.get(materialIndex).variants.push(reIndexedVariants.get(variantData.index));
        }
        const mappingsDef = Array.from(mappingTable.values())
            .map((m => { return m.variants.sort((a, b) => a - b) && m; }))
            .sort((a, b) => a.material - b.material);
        if (mappingsDef.length === 0) {
            return;
        }
        const originalMaterialIndex = compatibleMaterial(userData.originalMaterial) ?
            this.writer.processMaterial(userData.originalMaterial) :
            -1;
        for (const primitiveDef of meshDef.primitives) {
            // Override primitiveDef.material with original material.
            if (originalMaterialIndex >= 0) {
                primitiveDef.material = originalMaterialIndex;
            }
            primitiveDef.extensions = primitiveDef.extensions || {};
            primitiveDef.extensions[this.name] = { mappings: mappingsDef };
        }
    }
    afterParse() {
        if (this.variantNames.length === 0) {
            return;
        }
        const root = this.writer.json;
        root.extensions = root.extensions || {};
        const variantsDef = this.variantNames.map(n => {
            return { name: n };
        });
        root.extensions[this.name] = { variants: variantsDef };
        this.writer.extensionsUsed[this.name] = true;
    }
}
//# sourceMappingURL=VariantMaterialExporterPlugin.js.map