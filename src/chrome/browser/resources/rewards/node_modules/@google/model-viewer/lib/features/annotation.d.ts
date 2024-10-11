import ModelViewerElementBase, { Vector2D, Vector3D } from '../model-viewer-base.js';
import { HotspotConfiguration } from '../three-components/Hotspot.js';
import { Constructor } from '../utilities.js';
export declare type HotspotData = {
    position: Vector3D;
    normal: Vector3D;
    canvasPosition: Vector3D;
    facingCamera: boolean;
};
export declare interface AnnotationInterface {
    updateHotspot(config: HotspotConfiguration): void;
    queryHotspot(name: string): HotspotData | null;
    positionAndNormalFromPoint(pixelX: number, pixelY: number): {
        position: Vector3D;
        normal: Vector3D;
        uv: Vector2D | null;
    } | null;
}
/**
 * AnnotationMixin implements a declarative API to add hotspots and annotations.
 * Child elements of the <model-viewer> element that have a slot name that
 * begins with "hotspot" and data-position and data-normal attributes in
 * the format of the camera-target attribute will be added to the scene and
 * track the specified model coordinates.
 */
export declare const AnnotationMixin: <T extends Constructor<ModelViewerElementBase, object>>(ModelViewerElement: T) => {
    new (...args: any[]): AnnotationInterface;
    prototype: AnnotationInterface;
} & object & T;
