import { Vector3 } from 'three';
import { CSS2DObject } from 'three/examples/jsm/renderers/CSS2DRenderer.js';
export interface HotspotVisibilityDetails {
    visible: boolean;
}
/**
 * Hotspots are configured by slot name, and this name must begin with "hotspot"
 * to be recognized. The position and normal strings are in the form of the
 * camera-target attribute and default to "0m 0m 0m" and "0m 1m 0m",
 * respectively.
 */
export interface HotspotConfiguration {
    name: string;
    position?: string;
    normal?: string;
}
/**
 * The Hotspot object is a reference-counted slot. If decrement() returns true,
 * it should be removed from the tree so it can be garbage-collected.
 */
export declare class Hotspot extends CSS2DObject {
    normal: Vector3;
    private initialized;
    private referenceCount;
    private pivot;
    private slot;
    constructor(config: HotspotConfiguration);
    get facingCamera(): boolean;
    /**
     * Sets the hotspot to be in the highly visible foreground state.
     */
    show(): void;
    /**
     * Sets the hotspot to be in the diminished background state.
     */
    hide(): void;
    /**
     * Call this when adding elements to the same slot to keep track.
     */
    increment(): void;
    /**
     * Call this when removing elements from the slot; returns true when the slot
     * is unused.
     */
    decrement(): boolean;
    /**
     * Change the position of the hotspot to the input string, in the same format
     * as the data-position attribute.
     */
    updatePosition(position?: string): void;
    /**
     * Change the hotspot's normal to the input string, in the same format as the
     * data-normal attribute.
     */
    updateNormal(normal?: string): void;
    orient(radians: number): void;
    updateVisibility(show: boolean): void;
}
