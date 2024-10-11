import { Group } from 'three';
import { GLTF as ThreeGLTF, GLTFParser } from 'three/examples/jsm/loaders/GLTFLoader.js';
import { ExpressionNode, ExpressionTerm, FunctionNode, HexNode, IdentNode, Operator, OperatorNode } from '../styles/parsers.js';
import { PredicateFunction } from '../utilities.js';
export declare const elementFromLocalPoint: (document: Document | ShadowRoot, x: number, y: number) => Element | null;
export declare const pickShadowDescendant: (element: Element, x?: number, y?: number) => Element | null;
export declare const until: (predicate: PredicateFunction) => Promise<void>;
export declare const rafPasses: () => Promise<void>;
export interface SyntheticEventProperties {
    clientX?: number;
    clientY?: number;
    deltaY?: number;
    key?: string;
    shiftKey?: boolean;
}
/**
 * Dispatch a synthetic event on a given element with a given type, and
 * optionally with custom event properties. Returns the dispatched event.
 *
 * @param {HTMLElement} element
 * @param {type} string
 * @param {*} properties
 */
export declare const dispatchSyntheticEvent: (target: EventTarget, type: string, properties?: SyntheticEventProperties) => CustomEvent;
export declare const ASSETS_DIRECTORY = "../base/shared-assets/";
/**
 * Returns the full path for an asset by name. This is a convenience helper so
 * that we don't need to change paths throughout all test suites if we ever
 * decide to move files around.
 *
 * @param {string} name
 * @return {string}
 */
export declare const assetPath: (name: string) => string;
/**
 * Returns true if the given element is in the tree of the document of the
 * current frame.
 *
 * @param {HTMLElement} element
 * @return {boolean}
 */
export declare const isInDocumentTree: (node: Node) => boolean;
/**
 * "Spies" on a particular object by replacing a specified part of its
 * implementation with a custom version. Returns a function that restores the
 * original implementation to the object when invoked.
 */
export declare const spy: (object: Object, property: string, descriptor: PropertyDescriptor) => () => void;
/**
 * Helpers to assist in generating AST test fixtures
 */
export declare const expressionNode: (terms: Array<ExpressionTerm>) => ExpressionNode;
export declare const hexNode: (value: string) => HexNode;
export declare const identNode: (value: string) => IdentNode;
export declare const operatorNode: (value: Operator) => OperatorNode;
export declare const functionNode: (name: string, args: Array<ExpressionNode>) => FunctionNode;
export declare const loadThreeGLTF: (url: string) => Promise<ThreeGLTF>;
export declare const createFakeThreeGLTF: () => {
    animations: never[];
    scene: Group;
    scenes: Group[];
    cameras: never[];
    asset: {};
    parser: GLTFParser;
    userData: {};
};
