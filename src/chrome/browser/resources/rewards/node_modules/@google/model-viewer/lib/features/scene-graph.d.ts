import ModelViewerElementBase from '../model-viewer-base.js';
import { GLTF } from '../three-components/gltf-instance/gltf-defaulted.js';
import { Constructor } from '../utilities.js';
import { Material } from './scene-graph/material.js';
import { Model } from './scene-graph/model.js';
import { Texture as ModelViewerTexture } from './scene-graph/texture';
export declare const $currentGLTF: unique symbol;
export declare const $originalGltfJson: unique symbol;
export declare const $model: unique symbol;
interface SceneExportOptions {
    binary?: boolean;
    trs?: boolean;
    onlyVisible?: boolean;
    maxTextureSize?: number;
    includeCustomExtensions?: boolean;
    forceIndices?: boolean;
}
export interface SceneGraphInterface {
    readonly model?: Model;
    variantName: string | null;
    readonly availableVariants: string[];
    orientation: string;
    scale: string;
    readonly originalGltfJson: GLTF | null;
    exportScene(options?: SceneExportOptions): Promise<Blob>;
    createTexture(uri: string, type?: string): Promise<ModelViewerTexture | null>;
    /**
     * Intersects a ray with the scene and returns a list of materials who's
     * objects were intersected.
     * @param pixelX X coordinate of the mouse.
     * @param pixelY Y coordinate of the mouse.
     * @returns a material, if no intersection is made then null is returned.
     */
    materialFromPoint(pixelX: number, pixelY: number): Material | null;
}
/**
 * SceneGraphMixin manages exposes a model API in order to support operations on
 * the <model-viewer> scene graph.
 */
export declare const SceneGraphMixin: <T extends Constructor<ModelViewerElementBase, object>>(ModelViewerElement: T) => {
    new (...args: any[]): SceneGraphInterface;
    prototype: SceneGraphInterface;
} & object & T;
export {};
