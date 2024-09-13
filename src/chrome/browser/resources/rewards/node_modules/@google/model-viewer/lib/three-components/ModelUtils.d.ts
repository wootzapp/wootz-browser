import { Object3D, Vector3 } from 'three';
/**
 * Moves Three.js objects from one parent to another
 */
export declare const moveChildren: (from: Object3D, to: Object3D) => void;
/**
 * Performs a reduction across all the vertices of the input model and all its
 * children. The supplied function takes the reduced value and a vertex and
 * returns the newly reduced value. The value is initialized as zero.
 *
 * Adapted from Three.js, @see https://github.com/mrdoob/three.js/blob/7e0a78beb9317e580d7fa4da9b5b12be051c6feb/src/math/Box3.js#L241
 */
export declare const reduceVertices: <T>(model: Object3D, func: (value: T, vertex: Vector3) => T, initialValue: T) => T;
