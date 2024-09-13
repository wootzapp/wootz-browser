import { Group } from 'three';
import { GLTF } from 'three/examples/jsm/loaders/GLTFLoader.js';
import { Constructor } from '../utilities.js';
export declare const $prepared: unique symbol;
export interface PreparedGLTF extends GLTF {
    [$prepared]?: boolean;
}
export declare const $prepare: unique symbol;
export declare const $preparedGLTF: unique symbol;
export declare const $clone: unique symbol;
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
export declare class GLTFInstance implements GLTF {
    /**
     * Prepares a given GLTF for presentation and future cloning. A GLTF that is
     * prepared can safely have this method invoked on it multiple times; it will
     * only be prepared once, including after being cloned.
     */
    static prepare(source: GLTF): PreparedGLTF;
    /**
     * Override in an inheriting class to apply specialty one-time preparations
     * for a given input GLTF.
     */
    protected static [$prepare](source: GLTF): GLTF;
    protected [$preparedGLTF]: PreparedGLTF;
    get parser(): import("three/examples/jsm/loaders/GLTFLoader.js").GLTFParser;
    get animations(): import("three").AnimationClip[];
    get scene(): Group;
    get scenes(): Group[];
    get cameras(): import("three").Camera[];
    get asset(): {
        copyright?: string | undefined;
        generator?: string | undefined;
        version?: string | undefined;
        minVersion?: string | undefined;
        extensions?: any;
        extras?: any;
    };
    get userData(): any;
    constructor(preparedGLTF: PreparedGLTF);
    /**
     * Creates and returns a copy of this instance.
     */
    clone<T extends GLTFInstance>(): T;
    /**
     * Cleans up any retained memory that might not otherwise be released when
     * this instance is done being used.
     */
    dispose(): void;
    /**
     * Override in an inheriting class to implement specialized cloning strategies
     */
    protected [$clone](): PreparedGLTF;
}
export declare type GLTFInstanceConstructor = Constructor<GLTFInstance, {
    prepare: typeof GLTFInstance['prepare'];
}>;
