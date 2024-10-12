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
import { $threeTexture } from '../../../features/scene-graph/image.js';
import { $correlatedObjects } from '../../../features/scene-graph/three-dom-element.js';
import { ModelViewerElement } from '../../../model-viewer.js';
import { waitForEvent } from '../../../utilities.js';
import { assetPath } from '../../helpers.js';
const expect = chai.expect;
const ASTRONAUT_GLB_PATH = assetPath('models/Astronaut.glb');
suite('scene-graph/texture', () => {
    suite('Texture', () => {
        let element;
        let texture;
        setup(async () => {
            element = new ModelViewerElement();
            element.src = ASTRONAUT_GLB_PATH;
            document.body.insertBefore(element, document.body.firstChild);
            await waitForEvent(element, 'load');
            texture = await element.createTexture(assetPath('models/glTF-Sample-Models/2.0/BoxTextured/glTF/CesiumLogoFlat.png'));
            element.model.materials[0]
                .pbrMetallicRoughness.baseColorTexture.setTexture(texture);
        });
        teardown(() => {
            document.body.removeChild(element);
            texture = null;
        });
        test('Create a texture', async () => {
            expect(texture).to.not.be.null;
        });
        test('Set a texture', async () => {
            var _a, _b, _c;
            // Gets new UUID to compare with UUID of texture accessible through the
            // material.
            const newUUID = (_a = texture === null || texture === void 0 ? void 0 : texture.source[$threeTexture]) === null || _a === void 0 ? void 0 : _a.uuid;
            const threeTexture = (_c = (_b = element.model.materials[0]
                .pbrMetallicRoughness.baseColorTexture) === null || _b === void 0 ? void 0 : _b.texture) === null || _c === void 0 ? void 0 : _c.source[$threeTexture];
            expect(threeTexture.uuid).to.be.equal(newUUID);
        });
        test('Verify legacy correlatedObjects are updated.', async () => {
            var _a, _b, _c, _d, _e, _f, _g, _h, _j, _k;
            const newUUID = (_a = texture === null || texture === void 0 ? void 0 : texture.source[$threeTexture]) === null || _a === void 0 ? void 0 : _a.uuid;
            const uuidFromTextureObject = ((_d = (_c = (_b = element.model.materials[0]
                .pbrMetallicRoughness.baseColorTexture) === null || _b === void 0 ? void 0 : _b.texture) === null || _c === void 0 ? void 0 : _c[$correlatedObjects]) === null || _d === void 0 ? void 0 : _d.values().next().value)
                .uuid;
            expect(uuidFromTextureObject).to.be.equal(newUUID);
            const uuidFromSamplerObject = ((_g = (_f = (_e = element.model.materials[0]
                .pbrMetallicRoughness.baseColorTexture) === null || _e === void 0 ? void 0 : _e.texture) === null || _f === void 0 ? void 0 : _f.sampler[$correlatedObjects]) === null || _g === void 0 ? void 0 : _g.values().next().value)
                .uuid;
            expect(uuidFromSamplerObject).to.be.equal(newUUID);
            const uuidFromImageObject = ((_k = (_j = (_h = element.model.materials[0]
                .pbrMetallicRoughness.baseColorTexture) === null || _h === void 0 ? void 0 : _h.texture) === null || _j === void 0 ? void 0 : _j.source[$correlatedObjects]) === null || _k === void 0 ? void 0 : _k.values().next().value)
                .uuid;
            expect(uuidFromImageObject).to.be.equal(newUUID);
        });
    });
});
//# sourceMappingURL=texture-spec.js.map