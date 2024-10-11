import { Material, Object3D, Texture } from 'three';
import { GLTF as ThreeGLTF, GLTFReference } from 'three/examples/jsm/loaders/GLTFLoader.js';
import { GLTF, GLTFElement } from '../../three-components/gltf-instance/gltf-2.0.js';
export declare type ThreeSceneObject = Object3D | Material | Texture;
export declare type ThreeObjectSet = Set<ThreeSceneObject>;
export declare type GLTFElementToThreeObjectMap = Map<GLTFElement, ThreeObjectSet>;
export declare type ThreeObjectToGLTFElementHandleMap = Map<ThreeSceneObject, GLTFReference>;
declare const $threeGLTF: unique symbol;
declare const $gltf: unique symbol;
declare const $gltfElementMap: unique symbol;
declare const $threeObjectMap: unique symbol;
declare const $parallelTraverseThreeScene: unique symbol;
declare const $correlateOriginalThreeGLTF: unique symbol;
declare const $correlateCloneThreeGLTF: unique symbol;
/**
 * The Three.js GLTFLoader provides us with an in-memory representation
 * of a glTF in terms of Three.js constructs. It also provides us with a copy
 * of the deserialized glTF without any Three.js decoration, and a mapping of
 * glTF elements to their corresponding Three.js constructs.
 *
 * A CorrelatedSceneGraph exposes a synchronously available mapping of glTF
 * element references to their corresponding Three.js constructs.
 */
export declare class CorrelatedSceneGraph {
    /**
     * Produce a CorrelatedSceneGraph from a naturally generated Three.js GLTF.
     * Such GLTFs are produced by Three.js' GLTFLoader, and contain cached
     * details that expedite the correlation step.
     *
     * If a CorrelatedSceneGraph is provided as the second argument, re-correlates
     * a cloned Three.js GLTF with a clone of the glTF hierarchy used to produce
     * the upstream Three.js GLTF that the clone was created from. The result
     * CorrelatedSceneGraph is representative of the cloned hierarchy.
     */
    static from(threeGLTF: ThreeGLTF, upstreamCorrelatedSceneGraph?: CorrelatedSceneGraph): CorrelatedSceneGraph;
    private static [$correlateOriginalThreeGLTF];
    /**
     * Transfers the association between a raw glTF and a Three.js scene graph
     * to a clone of the Three.js scene graph, resolved as a new
     * CorrelatedSceneGraph instance.
     */
    private static [$correlateCloneThreeGLTF];
    /**
     * Traverses two presumably identical Three.js scenes, and invokes a
     * callback for each Object3D or Material encountered, including the initial
     * scene. Adapted from
     * https://github.com/mrdoob/three.js/blob/7c1424c5819ab622a346dd630ee4e6431388021e/examples/jsm/utils/SkeletonUtils.js#L586-L596
     */
    private static [$parallelTraverseThreeScene];
    private [$threeGLTF];
    private [$gltf];
    private [$gltfElementMap];
    private [$threeObjectMap];
    /**
     * The source Three.js GLTF result given to us by a Three.js GLTFLoader.
     */
    get threeGLTF(): ThreeGLTF;
    /**
     * The in-memory deserialized source glTF.
     */
    get gltf(): GLTF;
    /**
     * A Map of glTF element references to arrays of corresponding Three.js
     * object references. Three.js objects are kept in arrays to account for
     * cases where more than one Three.js object corresponds to a single glTF
     * element.
     */
    get gltfElementMap(): GLTFElementToThreeObjectMap;
    /**
     * A map of individual Three.js objects to corresponding elements in the
     * source glTF.
     */
    get threeObjectMap(): ThreeObjectToGLTFElementHandleMap;
    constructor(threeGLTF: ThreeGLTF, gltf: GLTF, threeObjectMap: ThreeObjectToGLTFElementHandleMap, gltfElementMap: GLTFElementToThreeObjectMap);
}
export {};
