import { Mesh, OrthographicCamera, PlaneGeometry, Scene, ShaderMaterial, Texture, WebGLRenderTarget } from 'three';
import { Constructor } from '../utilities.js';
import { ModelScene } from './ModelScene.js';
import { Renderer } from './Renderer.js';
export interface ModelViewerRendererDebugDetails {
    renderer: Renderer;
    THREE: {
        ShaderMaterial: Constructor<ShaderMaterial>;
        PlaneGeometry: Constructor<PlaneGeometry>;
        OrthographicCamera: Constructor<OrthographicCamera>;
        WebGLRenderTarget: Constructor<WebGLRenderTarget>;
        Texture: Constructor<Texture>;
        Scene: Constructor<Scene>;
        Mesh: Constructor<Mesh>;
    };
}
export interface ModelViewerSceneDetails {
    scene: ModelScene;
}
/**
 * This Debugger exposes internal details of the <model-viewer> rendering
 * substructure so that external tools can more easily inspect and operate on
 * them.
 *
 * It also activates shader debugging on the associated GL context. Shader
 * debugging trades performance for useful error information, so it is not
 * recommended to activate this unless needed.
 */
export declare class Debugger {
    constructor(renderer: Renderer);
    addScene(scene: ModelScene): void;
    removeScene(scene: ModelScene): void;
}
