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
import { $lazyLoadGLTFInfo } from '../../../features/scene-graph/material.js';
import { $correlatedObjects } from '../../../features/scene-graph/three-dom-element.js';
import { ModelViewerElement } from '../../../model-viewer.js';
import { waitForEvent } from '../../../utilities.js';
import { assetPath } from '../../helpers.js';
const expect = chai.expect;
const CUBES_GLTF_PATH = assetPath('models/cubes.gltf');
const HELMET_GLB_PATH = assetPath('models/glTF-Sample-Models/2.0/DamagedHelmet/glTF-Binary/DamagedHelmet.glb');
const ALPHA_BLEND_MODE_TEST = assetPath('models/glTF-Sample-Models/2.0/AlphaBlendModeTest/glTF-Binary/AlphaBlendModeTest.glb');
const REPLACEMENT_TEXTURE_PATH = assetPath('models/glTF-Sample-Models/2.0/BoxTextured/glTF/CesiumLogoFlat.png');
suite('scene-graph/material', () => {
    suite('Test Texture Slots', () => {
        let element;
        let texture;
        let threeMaterials;
        setup(async () => {
            element = new ModelViewerElement();
            element.src = HELMET_GLB_PATH;
            document.body.insertBefore(element, document.body.firstChild);
            await waitForEvent(element, 'load');
            texture = await element.createTexture(REPLACEMENT_TEXTURE_PATH);
            threeMaterials = element.model.materials[0][$correlatedObjects];
        });
        teardown(() => {
            document.body.removeChild(element);
            texture = null;
        });
        test('Set a new base map', async () => {
            var _a, _b;
            element.model.materials[0]
                .pbrMetallicRoughness.baseColorTexture.setTexture(texture);
            // Gets new UUID to compare with UUID of texture accessible through the
            // material.
            const newUUID = texture === null || texture === void 0 ? void 0 : texture.source[$threeTexture].uuid;
            const threeTexture = (_b = (_a = element.model.materials[0]
                .pbrMetallicRoughness.baseColorTexture) === null || _a === void 0 ? void 0 : _a.texture) === null || _b === void 0 ? void 0 : _b.source[$threeTexture];
            for (const material of threeMaterials) {
                expect(material.map).to.be.eq(threeTexture);
            }
            expect(threeTexture.uuid).to.be.equal(newUUID);
        });
        test('Set a new metallicRoughness map', async () => {
            var _a, _b, _c;
            element.model.materials[0]
                .pbrMetallicRoughness.metallicRoughnessTexture.setTexture(texture);
            // Gets new UUID to compare with UUID of texture accessible through the
            // material.
            const newUUID = (_a = texture === null || texture === void 0 ? void 0 : texture.source[$threeTexture]) === null || _a === void 0 ? void 0 : _a.uuid;
            const threeTexture = (_c = (_b = element.model.materials[0]
                .pbrMetallicRoughness.metallicRoughnessTexture) === null || _b === void 0 ? void 0 : _b.texture) === null || _c === void 0 ? void 0 : _c.source[$threeTexture];
            for (const material of threeMaterials) {
                expect(material.metalnessMap).to.be.eq(threeTexture);
                expect(material.roughnessMap).to.be.eq(threeTexture);
            }
            expect(threeTexture.uuid).to.be.equal(newUUID);
        });
        test('Set a new normal map', async () => {
            var _a, _b, _c;
            element.model.materials[0].normalTexture.setTexture(texture);
            // Gets new UUID to compare with UUID of texture accessible through the
            // material.
            const newUUID = (_a = texture === null || texture === void 0 ? void 0 : texture.source[$threeTexture]) === null || _a === void 0 ? void 0 : _a.uuid;
            const threeTexture = (_c = (_b = element.model.materials[0]
                .normalTexture) === null || _b === void 0 ? void 0 : _b.texture) === null || _c === void 0 ? void 0 : _c.source[$threeTexture];
            for (const material of threeMaterials) {
                expect(material.normalMap).to.be.eq(threeTexture);
            }
            expect(threeTexture.uuid).to.be.equal(newUUID);
        });
        test('Set a new occlusion map', async () => {
            var _a, _b, _c;
            element.model.materials[0].occlusionTexture.setTexture(texture);
            // Gets new UUID to compare with UUID of texture accessible through the
            // material.
            const newUUID = (_a = texture === null || texture === void 0 ? void 0 : texture.source[$threeTexture]) === null || _a === void 0 ? void 0 : _a.uuid;
            const threeTexture = (_c = (_b = element.model.materials[0]
                .occlusionTexture) === null || _b === void 0 ? void 0 : _b.texture) === null || _c === void 0 ? void 0 : _c.source[$threeTexture];
            for (const material of threeMaterials) {
                expect(material.aoMap).to.be.eq(threeTexture);
            }
            expect(threeTexture.uuid).to.be.equal(newUUID);
        });
        test('Set a new emissive map', async () => {
            var _a, _b, _c;
            element.model.materials[0].emissiveTexture.setTexture(texture);
            // Gets new UUID to compare with UUID of texture accessible through the
            // material.
            const newUUID = (_a = texture === null || texture === void 0 ? void 0 : texture.source[$threeTexture]) === null || _a === void 0 ? void 0 : _a.uuid;
            const threeTexture = (_c = (_b = element.model.materials[0]
                .emissiveTexture) === null || _b === void 0 ? void 0 : _b.texture) === null || _c === void 0 ? void 0 : _c.source[$threeTexture];
            for (const material of threeMaterials) {
                expect(material.emissiveMap).to.be.eq(threeTexture);
            }
            expect(threeTexture.uuid).to.be.equal(newUUID);
        });
    });
    suite('Material properties', () => {
        let element;
        setup(async () => {
            element = new ModelViewerElement();
        });
        teardown(() => {
            document.body.removeChild(element);
        });
        const loadModel = async (path) => {
            element.src = path;
            document.body.insertBefore(element, document.body.firstChild);
            await waitForEvent(element, 'load');
        };
        test('test alpha cutoff expect disabled by default', async () => {
            var _a;
            await loadModel(HELMET_GLB_PATH);
            expect((_a = element.model.materials[0][$correlatedObjects]) === null || _a === void 0 ? void 0 : _a.values().next().value.alphaTest)
                .to.be.equal(0);
        });
        test('test alpha cutoff expect valid value as default', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            expect(element.model.materials[2].getAlphaCutoff()).to.be.equal(0.25);
        });
        test('test alpha cutoff test setting and getting', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            element.model.materials[2].setAlphaCutoff(0.5);
            expect(element.model.materials[2].getAlphaCutoff()).to.be.equal(0.5);
        });
        test('test double sided expect default is false', async () => {
            await loadModel(HELMET_GLB_PATH);
            expect(element.model.materials[0].getDoubleSided()).to.be.equal(false);
        });
        test('test double sided expect default is true', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            expect(element.model.materials[1].getDoubleSided()).to.be.equal(true);
        });
        test('test double sided setting and getting', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            expect(element.model.materials[1].getDoubleSided()).to.be.equal(true);
            element.model.materials[1].setDoubleSided(false);
            expect(element.model.materials[1].getDoubleSided()).to.be.equal(false);
            element.model.materials[1].setDoubleSided(true);
            expect(element.model.materials[1].getDoubleSided()).to.be.equal(true);
        });
        test('test alpha-mode, setting and getting', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            element.model.materials[0].setAlphaMode('BLEND');
            expect(element.model.materials[0].getAlphaMode()).to.be.equal('BLEND');
            element.model.materials[0].setAlphaMode('MASK');
            expect(element.model.materials[0].getAlphaMode()).to.be.equal('MASK');
            element.model.materials[0].setAlphaMode('OPAQUE');
            expect(element.model.materials[0].getAlphaMode()).to.be.equal('OPAQUE');
        });
        test('test alpha-mode, expect default of opaque', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            expect(element.model.materials[0].getAlphaMode()).to.be.equal('OPAQUE');
        });
        test('test alpha-mode, expect default of blend', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            expect(element.model.materials[1].getAlphaMode()).to.be.equal('BLEND');
        });
        test('test alpha-mode, expect default of mask', async () => {
            await loadModel(ALPHA_BLEND_MODE_TEST);
            expect(element.model.materials[2].getAlphaMode()).to.be.equal('MASK');
        });
    });
    suite('Material lazy loading', () => {
        let element;
        let model;
        setup(async () => {
            element = new ModelViewerElement();
            element.src = CUBES_GLTF_PATH;
            document.body.insertBefore(element, document.body.firstChild);
            await waitForEvent(element, 'load');
            model = element.model;
        });
        teardown(() => {
            document.body.removeChild(element);
        });
        test('Accessing the name getter does not cause throw error.', async () => {
            expect(model.materials[2].name).to.equal('red');
            expect(model.materials[2][$lazyLoadGLTFInfo]).to.be.ok;
        });
        test('Accessing a getter of an unloaded material throws an error.', async () => {
            expect(() => { model.materials[2].pbrMetallicRoughness; }).to.throw;
            expect(model.materials[2].isLoaded).to.be.false;
        });
        test('Accessing a getter of a loaded material has valid data.', async () => {
            await model.materials[2].ensureLoaded();
            expect(model.materials[2].isLoaded).to.be.true;
            const pbr = model.materials[2].pbrMetallicRoughness;
            expect(pbr).to.be.ok;
        });
    });
});
//# sourceMappingURL=material-spec.js.map