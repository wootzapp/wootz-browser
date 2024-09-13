/* @license
 * Copyright 2019 Google LLC. All Rights Reserved.
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
import { Group } from 'three';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader.js';
import { deserializeUrl, timePasses } from '../utilities.js';
export const elementFromLocalPoint = (document, x, y) => {
    const host = (document === window.document) ?
        window.document.body :
        document.host;
    const actualDocument = window.ShadyCSS ? window.document : document;
    const boundingRect = host.getBoundingClientRect();
    return actualDocument.elementFromPoint(boundingRect.left + x, boundingRect.top + y);
};
export const pickShadowDescendant = (element, x = 0, y = 0) => {
    return element.shadowRoot != null ?
        elementFromLocalPoint(element.shadowRoot, x, y) :
        null;
};
export const until = async (predicate) => {
    while (!predicate()) {
        await timePasses();
    }
};
export const rafPasses = () => new Promise(resolve => requestAnimationFrame(() => resolve()));
/**
 * Dispatch a synthetic event on a given element with a given type, and
 * optionally with custom event properties. Returns the dispatched event.
 *
 * @param {HTMLElement} element
 * @param {type} string
 * @param {*} properties
 */
export const dispatchSyntheticEvent = (target, type, properties = {
    clientX: 0,
    clientY: 0,
    deltaY: 1.0
}) => {
    const event = new CustomEvent(type, { cancelable: true, bubbles: true });
    Object.assign(event, properties);
    target.dispatchEvent(event);
    return event;
};
export const ASSETS_DIRECTORY = '../base/shared-assets/';
/**
 * Returns the full path for an asset by name. This is a convenience helper so
 * that we don't need to change paths throughout all test suites if we ever
 * decide to move files around.
 *
 * @param {string} name
 * @return {string}
 */
export const assetPath = (name) => deserializeUrl(`${ASSETS_DIRECTORY}${name}`);
/**
 * Returns true if the given element is in the tree of the document of the
 * current frame.
 *
 * @param {HTMLElement} element
 * @return {boolean}
 */
export const isInDocumentTree = (node) => {
    let root = node.getRootNode();
    while (root !== node && root != null) {
        if (root.nodeType === Node.DOCUMENT_NODE) {
            return root === document;
        }
        root = root.host && root.host.getRootNode();
    }
    return false;
};
/**
 * "Spies" on a particular object by replacing a specified part of its
 * implementation with a custom version. Returns a function that restores the
 * original implementation to the object when invoked.
 */
export const spy = (object, property, descriptor) => {
    let sourcePrototype = object;
    while (sourcePrototype != null &&
        !sourcePrototype.hasOwnProperty(property)) {
        sourcePrototype = sourcePrototype.__proto__;
    }
    if (sourcePrototype == null) {
        throw new Error(`Cannot spy property "${property}" on ${object}`);
    }
    const originalDescriptor = Object.getOwnPropertyDescriptor(sourcePrototype, property);
    if (originalDescriptor == null) {
        throw new Error(`Cannot read descriptor of "${property}" on ${object}`);
    }
    Object.defineProperty(sourcePrototype, property, descriptor);
    return () => {
        Object.defineProperty(sourcePrototype, property, originalDescriptor);
    };
};
/**
 * Helpers to assist in generating AST test fixtures
 */
export const expressionNode = (terms) => ({ type: 'expression', terms });
export const hexNode = (value) => ({ type: 'hex', value });
export const identNode = (value) => ({ type: 'ident', value });
export const operatorNode = (value) => ({ type: 'operator', value });
export const functionNode = (name, args) => ({ type: 'function', name: identNode(name), arguments: args });
export const loadThreeGLTF = (url) => {
    const loader = new GLTFLoader();
    return new Promise((resolve, reject) => {
        loader.load(url, resolve, undefined, reject);
    });
};
export const createFakeThreeGLTF = () => {
    const scene = new Group();
    return {
        animations: [],
        scene,
        scenes: [scene],
        cameras: [],
        asset: {},
        parser: {
            cache: new Map(),
            json: { scene: 0, scenes: [{}], materials: [], nodes: [] },
            associations: new Map()
        },
        userData: {}
    };
};
//# sourceMappingURL=helpers.js.map