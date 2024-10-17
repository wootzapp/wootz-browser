import { Material, Object3D, Texture } from 'three';
import { GLTFElement } from '../../three-components/gltf-instance/gltf-defaulted.js';
export declare const $correlatedObjects: unique symbol;
export declare const $sourceObject: unique symbol;
export declare const $onUpdate: unique symbol;
declare type CorrelatedObjects = Set<Object3D> | Set<Material> | Set<Texture>;
/**
 * A SerializableThreeDOMElement is the common primitive of all scene graph
 * elements that have been facaded in the host execution context. It adds
 * a common interface to these elements in support of convenient
 * serializability.
 */
export declare class ThreeDOMElement {
    readonly [$onUpdate]: () => void;
    readonly [$sourceObject]: GLTFElement;
    [$correlatedObjects]: CorrelatedObjects | null;
    constructor(onUpdate: () => void, element: GLTFElement, correlatedObjects?: CorrelatedObjects | null);
}
export {};
