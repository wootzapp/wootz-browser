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
import { FrontSide, Sphere } from 'three';
import { $clone, $prepare, $preparedGLTF, GLTFInstance } from '../GLTFInstance.js';
import { CorrelatedSceneGraph } from './correlated-scene-graph.js';
const $correlatedSceneGraph = Symbol('correlatedSceneGraph');
/**
 * This specialization of GLTFInstance collects all of the processing needed
 * to prepare a model and to clone it making special considerations for
 * <model-viewer> use cases.
 */
export class ModelViewerGLTFInstance extends GLTFInstance {
    /**
     * @override
     */
    static [$prepare](source) {
        const prepared = super[$prepare](source);
        if (prepared[$correlatedSceneGraph] == null) {
            prepared[$correlatedSceneGraph] = CorrelatedSceneGraph.from(prepared);
        }
        const { scene } = prepared;
        const nullSphere = new Sphere(undefined, Infinity);
        scene.traverse((node) => {
            // Set a high renderOrder while we're here to ensure the model
            // always renders on top of the sky sphere
            node.renderOrder = 1000;
            // Three.js seems to cull some animated models incorrectly. Since we
            // expect to view our whole scene anyway, we turn off the frustum
            // culling optimization here.
            node.frustumCulled = false;
            // Animations for objects without names target their UUID instead. When
            // objects are cloned, they get new UUIDs which the animation can't
            // find. To fix this, we assign their UUID as their name.
            if (!node.name) {
                node.name = node.uuid;
            }
            const mesh = node;
            if (mesh.isMesh) {
                const { geometry } = mesh;
                mesh.castShadow = true;
                if (mesh.isSkinnedMesh) {
                    // Akin to disabling frustum culling above, we have to also manually
                    // disable the bounds to make raycasting correct for skinned meshes.
                    geometry.boundingSphere = nullSphere;
                    // The bounding box is set in GLTFLoader by the accessor bounds, which
                    // are not updated with animation.
                    geometry.boundingBox = null;
                }
                const material = mesh.material;
                if (material.isMeshBasicMaterial === true) {
                    material.toneMapped = false;
                }
                // This makes shadows better for non-manifold meshes
                material.shadowSide = FrontSide;
                // Fixes an edge case with unused extra UV-coords being incorrectly
                // referenced by three.js; remove when
                // https://github.com/mrdoob/three.js/pull/23974 is merged.
                if (material.aoMap) {
                    const { gltf, threeObjectMap } = prepared[$correlatedSceneGraph];
                    const gltfRef = threeObjectMap.get(material);
                    if (gltf.materials != null && gltfRef != null &&
                        gltfRef.materials != null) {
                        const gltfMaterial = gltf.materials[gltfRef.materials];
                        if (gltfMaterial.occlusionTexture &&
                            gltfMaterial.occlusionTexture.texCoord === 0 &&
                            geometry.attributes.uv != null) {
                            geometry.setAttribute('uv2', geometry.attributes.uv);
                        }
                    }
                }
            }
        });
        return prepared;
    }
    get correlatedSceneGraph() {
        return this[$preparedGLTF][$correlatedSceneGraph];
    }
    /**
     * @override
     */
    [$clone]() {
        const clone = super[$clone]();
        const sourceUUIDToClonedMaterial = new Map();
        clone.scene.traverse((node) => {
            // Materials aren't cloned when cloning meshes; geometry
            // and materials are copied by reference. This is necessary
            // for the same model to be used twice with different
            // scene-graph operations.
            if (node.isMesh) {
                const mesh = node;
                const material = mesh.material;
                if (material != null) {
                    if (sourceUUIDToClonedMaterial.has(material.uuid)) {
                        mesh.material = sourceUUIDToClonedMaterial.get(material.uuid);
                        return;
                    }
                    mesh.material = material.clone();
                    sourceUUIDToClonedMaterial.set(material.uuid, mesh.material);
                }
            }
        });
        // Cross-correlate the scene graph by relying on information in the
        // current scene graph; without this step, relationships between the
        // Three.js object graph and the glTF scene graph will be lost.
        clone[$correlatedSceneGraph] =
            CorrelatedSceneGraph.from(clone, this.correlatedSceneGraph);
        return clone;
    }
}
//# sourceMappingURL=ModelViewerGLTFInstance.js.map