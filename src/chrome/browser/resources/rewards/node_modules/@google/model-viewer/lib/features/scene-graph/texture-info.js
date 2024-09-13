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
var _a, _b;
import { LinearEncoding, sRGBEncoding, Vector2 } from 'three';
import { $threeTexture } from './image.js';
import { Texture } from './texture.js';
const $texture = Symbol('texture');
const $transform = Symbol('transform');
export const $materials = Symbol('materials');
export const $usage = Symbol('usage');
// Defines what a texture will be used for.
export var TextureUsage;
(function (TextureUsage) {
    TextureUsage[TextureUsage["Base"] = 0] = "Base";
    TextureUsage[TextureUsage["MetallicRoughness"] = 1] = "MetallicRoughness";
    TextureUsage[TextureUsage["Normal"] = 2] = "Normal";
    TextureUsage[TextureUsage["Occlusion"] = 3] = "Occlusion";
    TextureUsage[TextureUsage["Emissive"] = 4] = "Emissive";
})(TextureUsage || (TextureUsage = {}));
/**
 * TextureInfo facade implementation for Three.js materials
 */
export class TextureInfo {
    constructor(onUpdate, usage, threeTexture, material, gltf, gltfTextureInfo) {
        this[_a] = null;
        this[_b] = {
            rotation: 0,
            scale: new Vector2(1, 1),
            offset: new Vector2(0, 0)
        };
        // Creates image, sampler, and texture if valid texture info is provided.
        if (gltfTextureInfo && threeTexture) {
            const gltfTexture = gltf.textures ? gltf.textures[gltfTextureInfo.index] : null;
            const sampler = gltfTexture ?
                (gltf.samplers ? gltf.samplers[gltfTexture.sampler] : null) :
                null;
            const image = gltfTexture ?
                (gltf.images ? gltf.images[gltfTexture.source] : null) :
                null;
            this[$transform].rotation = threeTexture.rotation;
            this[$transform].scale.copy(threeTexture.repeat);
            this[$transform].offset.copy(threeTexture.offset);
            this[$texture] =
                new Texture(onUpdate, threeTexture, gltfTexture, sampler, image);
        }
        this.onUpdate = onUpdate;
        this[$materials] = material;
        this[$usage] = usage;
    }
    get texture() {
        return this[$texture];
    }
    setTexture(texture) {
        const threeTexture = texture != null ? texture.source[$threeTexture] : null;
        let encoding = sRGBEncoding;
        this[$texture] = texture;
        if (this[$materials]) {
            for (const material of this[$materials]) {
                switch (this[$usage]) {
                    case TextureUsage.Base:
                        material.map = threeTexture;
                        break;
                    case TextureUsage.MetallicRoughness:
                        encoding = LinearEncoding;
                        material.metalnessMap = threeTexture;
                        material.roughnessMap = threeTexture;
                        break;
                    case TextureUsage.Normal:
                        encoding = LinearEncoding;
                        material.normalMap = threeTexture;
                        break;
                    case TextureUsage.Occlusion:
                        encoding = LinearEncoding;
                        material.aoMap = threeTexture;
                        break;
                    case TextureUsage.Emissive:
                        material.emissiveMap = threeTexture;
                        break;
                    default:
                }
                material.needsUpdate = true;
            }
        }
        if (threeTexture) {
            // Updates the encoding for the texture, affects all references.
            threeTexture.encoding = encoding;
            threeTexture.rotation = this[$transform].rotation;
            threeTexture.repeat = this[$transform].scale;
            threeTexture.offset = this[$transform].offset;
        }
        this.onUpdate();
    }
}
_a = $texture, _b = $transform;
//# sourceMappingURL=texture-info.js.map