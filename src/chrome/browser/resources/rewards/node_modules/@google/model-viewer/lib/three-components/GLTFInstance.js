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
import { Texture } from 'three';
import * as SkeletonUtils from 'three/examples/jsm/utils/SkeletonUtils.js';
export const $prepared = Symbol('prepared');
export const $prepare = Symbol('prepare');
export const $preparedGLTF = Symbol('preparedGLTF');
export const $clone = Symbol('clone');
/**
 * Represents the preparation and enhancement of the output of a Three.js
 * GLTFLoader (a Three.js-flavor "GLTF"), to make it suitable for optimal,
 * correct viewing in a given presentation context and also make the cloning
 * process more explicit and legible.
 *
 * A GLTFInstance is API-compatible with a Three.js-flavor "GLTF", so it should
 * be considered to be interchangeable with the loaded result of a GLTFLoader.
 *
 * This basic implementation only implements trivial preparation and enhancement
 * of a GLTF. These operations are intended to be enhanced by inheriting
 * classes.
 */
export class GLTFInstance {
    constructor(preparedGLTF) {
        this[$preparedGLTF] = preparedGLTF;
    }
    /**
     * Prepares a given GLTF for presentation and future cloning. A GLTF that is
     * prepared can safely have this method invoked on it multiple times; it will
     * only be prepared once, including after being cloned.
     */
    static prepare(source) {
        if (source.scene == null) {
            throw new Error('Model does not have a scene');
        }
        if (source[$prepared]) {
            return source;
        }
        const prepared = this[$prepare](source);
        // NOTE: ES5 Symbol polyfill is not compatible with spread operator
        // so {...prepared, [$prepared]: true} does not work
        prepared[$prepared] = true;
        return prepared;
    }
    /**
     * Override in an inheriting class to apply specialty one-time preparations
     * for a given input GLTF.
     */
    static [$prepare](source) {
        // TODO(#195,#1003): We don't currently support multiple scenes, so we don't
        // bother preparing extra scenes for now:
        const { scene } = source;
        const scenes = [scene];
        return Object.assign(Object.assign({}, source), { scene, scenes });
    }
    get parser() {
        return this[$preparedGLTF].parser;
    }
    get animations() {
        return this[$preparedGLTF].animations;
    }
    get scene() {
        return this[$preparedGLTF].scene;
    }
    get scenes() {
        return this[$preparedGLTF].scenes;
    }
    get cameras() {
        return this[$preparedGLTF].cameras;
    }
    get asset() {
        return this[$preparedGLTF].asset;
    }
    get userData() {
        return this[$preparedGLTF].userData;
    }
    /**
     * Creates and returns a copy of this instance.
     */
    clone() {
        const GLTFInstanceConstructor = this.constructor;
        const clonedGLTF = this[$clone]();
        return new GLTFInstanceConstructor(clonedGLTF);
    }
    /**
     * Cleans up any retained memory that might not otherwise be released when
     * this instance is done being used.
     */
    dispose() {
        this.scenes.forEach((scene) => {
            scene.traverse((object) => {
                if (!object.isMesh) {
                    return;
                }
                const mesh = object;
                const materials = Array.isArray(mesh.material) ? mesh.material : [mesh.material];
                materials.forEach(material => {
                    // Explicitly dispose any textures assigned to this material
                    for (const propertyName in material) {
                        const texture = material[propertyName];
                        if (texture instanceof Texture) {
                            const image = texture.source.data;
                            if (image.close != null) {
                                image.close();
                            }
                            texture.dispose();
                        }
                    }
                    material.dispose();
                });
                mesh.geometry.dispose();
            });
        });
    }
    /**
     * Override in an inheriting class to implement specialized cloning strategies
     */
    [$clone]() {
        const source = this[$preparedGLTF];
        // TODO(#195,#1003): We don't currently support multiple scenes, so we don't
        // bother cloning extra scenes for now:
        const scene = SkeletonUtils.clone(this.scene);
        cloneVariantMaterials(scene, this.scene);
        const scenes = [scene];
        const userData = source.userData ? Object.assign({}, source.userData) : {};
        return Object.assign(Object.assign({}, source), { scene, scenes, userData });
    }
}
// Variant materials and original material instances are stored under
// object.userData.variantMaterials/originalMaterial.
// Three.js Object3D.clone() doesn't clone Three.js objects under
// .userData so this function is a workaround.
const cloneVariantMaterials = (dst, src) => {
    traversePair(dst, src, (dst, src) => {
        if (src.userData.variantMaterials !== undefined) {
            dst.userData.variantMaterials = new Map(src.userData.variantMaterials);
        }
        if (src.userData.variantData !== undefined) {
            dst.userData.variantData = src.userData.variantData;
        }
        if (src.userData.originalMaterial !== undefined) {
            dst.userData.originalMaterial = src.userData.originalMaterial;
        }
    });
};
const traversePair = (obj1, obj2, callback) => {
    callback(obj1, obj2);
    // Assume obj1 and obj2 have the same tree structure
    for (let i = 0; i < obj1.children.length; i++) {
        traversePair(obj1.children[i], obj2.children[i], callback);
    }
};
//# sourceMappingURL=GLTFInstance.js.map