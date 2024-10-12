import { GLTF, GLTFElement, GLTFElementMap } from '../../three-components/gltf-instance/gltf-2.0.js';
/**
 * A VisitorCallback is used to access an element of a GLTF scene graph. The
 * callback receives a reference to the element, the element's index relative
 * to other elements of its type (as it would appear in the top-level element
 * array in the corresponding GLTF) and a hierarchy array that is the list of
 * ancestor elements (inclusive of the current element) that preceded the
 * currently accessed element in the GLTF scene graph hierarchy.
 */
export declare type VisitorCallback<T extends GLTFElement> = (element: T, index: number, hierarchy: GLTFElement[]) => void;
/**
 * There is a corresponding VisitorCallback type for every type of GLTFElement.
 */
export declare type VisitorCallbacks = {
    [T in keyof GLTFElementMap]?: VisitorCallback<GLTFElementMap[T]>;
};
/**
 * Supported options for configuring a GLTFTreeVisitor include:
 *  - allScenes: if true, all scenes (not just the active one) will be visited
 *  - sparse: if true, elements that have been visited once will not be visited
 *    again if and when they are encountered a second time in the scene graph
 *
 * Note that glTF defines a top-level field ("scene") that refers to the scene
 * that will be presented first ("at load time"). In the case of <model-viewer>,
 * there is no way to specify an alternative scene to display (although we plan
 * to add configuration for this). Consequently, the ability to traverse all
 * scenes is not likely to be used at this time. However, some cases will call
 * for visiting all nodes in a glTF regardless of whether they are a part of
 * the current scene. Eventually, <model-viewer> will support changing the
 * active scene, and the ability to traverse all scenes at once will become
 * handy.
 *
 * @see https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#scenes
 * @see https://github.com/google/model-viewer/issues/195
 */
export interface VisitOptions {
    allScenes?: boolean;
    sparse?: boolean;
}
declare const $callbacks: unique symbol;
declare const $visitMesh: unique symbol;
declare const $visitElement: unique symbol;
declare const $visitNode: unique symbol;
declare const $visitScene: unique symbol;
declare const $visitMaterial: unique symbol;
/**
 * GLTFTreeVisitor implements a deterministic traversal order of a valid,
 * deserialized glTF 2.0 model. It supports selective element visitation via
 * callbacks configured based on element type. For example, to visit all
 * materials in all scenes in a glTF:
 *
 * ```javascript
 * const visitor = new GLTFTreeVisitor({
 *   material: (material, index, hierarchy) => {
 *     // material is a glTF 2.0 Material
 *     // index is the index of material in gltf.materials
 *     // hierarchy includes ancestors of material in the glTF
 *   }
 * });
 *
 * visitor.visit(someInMemoryGLTF, { allScenes: true });
 * ```
 *
 * The traversal order of the visitor is pre-order, depth-first.
 *
 * Note that the traversal order is not guaranteed to correspond to the
 * index of a given element in any way. Rather, traversal order is based
 * on the hierarchical order of the scene graph.
 */
export declare class GLTFTreeVisitor {
    private [$callbacks];
    constructor(callbacks: VisitorCallbacks);
    /**
     * Visit a given in-memory glTF via the configured callbacks of this visitor.
     * Optionally, all scenes may be visited (as opposed to just the active one).
     * Sparse traversal can also be specified, in which case elements that
     * re-appear multiple times in the scene graph will only be visited once.
     */
    visit(gltf: GLTF, options?: VisitOptions): void;
    private [$visitElement];
    private [$visitScene];
    private [$visitNode];
    private [$visitMesh];
    private [$visitMaterial];
}
export {};
