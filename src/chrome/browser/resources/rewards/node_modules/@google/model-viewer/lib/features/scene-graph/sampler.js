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
import { Filter, Wrap } from '../../three-components/gltf-instance/gltf-2.0.js';
import { $correlatedObjects, $onUpdate, $sourceObject, ThreeDOMElement } from './three-dom-element.js';
const isMinFilter = (() => {
    const minFilterValues = [
        Filter.Nearest,
        Filter.Linear,
        Filter.NearestMipmapNearest,
        Filter.LinearMipmapLinear,
        Filter.NearestMipmapLinear,
        Filter.LinearMipmapLinear
    ];
    return (value) => minFilterValues.indexOf(value) > -1;
})();
const isMagFilter = (() => {
    const magFilterValues = [Filter.Nearest, Filter.Linear];
    return (value) => magFilterValues.indexOf(value) > -1;
})();
const isWrapMode = (() => {
    const wrapModes = [Wrap.ClampToEdge, Wrap.MirroredRepeat, Wrap.Repeat];
    return (value) => wrapModes.indexOf(value) > -1;
})();
const isValidSamplerValue = (property, value) => {
    switch (property) {
        case 'minFilter':
            return isMinFilter(value);
        case 'magFilter':
            return isMagFilter(value);
        case 'wrapS':
        case 'wrapT':
            return isWrapMode(value);
        default:
            throw new Error(`Cannot configure property "${property}" on Sampler`);
    }
};
const $threeTextures = Symbol('threeTextures');
const $setProperty = Symbol('setProperty');
const $sourceSampler = Symbol('sourceSampler');
/**
 * Sampler facade implementation for Three.js textures
 */
export class Sampler extends ThreeDOMElement {
    get [$threeTextures]() {
        console.assert(this[$correlatedObjects] != null && this[$correlatedObjects].size > 0, 'Sampler correlated object is undefined');
        return this[$correlatedObjects];
    }
    get [$sourceSampler]() {
        console.assert(this[$sourceObject] != null, 'Sampler source is undefined');
        return this[$sourceObject];
    }
    constructor(onUpdate, texture, gltfSampler) {
        gltfSampler = gltfSampler !== null && gltfSampler !== void 0 ? gltfSampler : {};
        // These defaults represent a convergence of glTF defaults for wrap mode and
        // Three.js defaults for filters. Per glTF 2.0 spec, a renderer may choose
        // its own defaults for filters.
        // @see https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#reference-sampler
        // @see https://threejs.org/docs/#api/en/textures/Texture
        if (gltfSampler.minFilter == null) {
            gltfSampler.minFilter =
                texture ? texture.minFilter : Filter.LinearMipmapLinear;
        }
        if (gltfSampler.magFilter == null) {
            gltfSampler.magFilter =
                texture ? texture.magFilter : Filter.Linear;
        }
        if (gltfSampler.wrapS == null) {
            gltfSampler.wrapS = texture ? texture.wrapS : Wrap.Repeat;
        }
        if (gltfSampler.wrapT == null) {
            gltfSampler.wrapT = texture ? texture.wrapT : Wrap.Repeat;
        }
        super(onUpdate, gltfSampler, new Set(texture ? [texture] : []));
    }
    get name() {
        return this[$sourceObject].name || '';
    }
    get minFilter() {
        return this[$sourceSampler].minFilter;
    }
    get magFilter() {
        return this[$sourceSampler].magFilter;
    }
    get wrapS() {
        return this[$sourceSampler].wrapS;
    }
    get wrapT() {
        return this[$sourceSampler].wrapT;
    }
    setMinFilter(filter) {
        this[$setProperty]('minFilter', filter);
    }
    setMagFilter(filter) {
        this[$setProperty]('magFilter', filter);
    }
    setWrapS(mode) {
        this[$setProperty]('wrapS', mode);
    }
    setWrapT(mode) {
        this[$setProperty]('wrapT', mode);
    }
    [$setProperty](property, value) {
        const sampler = this[$sourceSampler];
        if (sampler != null) {
            if (isValidSamplerValue(property, value)) {
                sampler[property] = value;
                for (const texture of this[$threeTextures]) {
                    texture[property] = value;
                    texture.needsUpdate = true;
                }
            }
            this[$onUpdate]();
        }
    }
}
//# sourceMappingURL=sampler.js.map