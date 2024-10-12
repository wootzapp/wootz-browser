import ModelViewerElementBase from './model-viewer-base.js';
export declare const ModelViewerElement: {
    new (...args: any[]): import("./features/annotation.js").AnnotationInterface;
    prototype: import("./features/annotation.js").AnnotationInterface;
} & object & {
    new (...args: any[]): import("./features/scene-graph.js").SceneGraphInterface;
    prototype: import("./features/scene-graph.js").SceneGraphInterface;
} & {
    new (...args: any[]): import("./features/staging.js").StagingInterface;
    prototype: import("./features/staging.js").StagingInterface;
} & {
    new (...args: any[]): import("./features/environment.js").EnvironmentInterface;
    prototype: import("./features/environment.js").EnvironmentInterface;
} & {
    new (...args: any[]): import("./features/controls.js").ControlsInterface;
    prototype: import("./features/controls.js").ControlsInterface;
} & {
    new (...args: any[]): import("./features/ar.js").ARInterface;
    prototype: import("./features/ar.js").ARInterface;
} & {
    new (...args: any[]): import("./features/loading.js").LoadingInterface;
    prototype: import("./features/loading.js").LoadingInterface;
} & import("./features/loading.js").LoadingStaticInterface & {
    new (...args: any[]): import("./features/animation.js").AnimationInterface;
    prototype: import("./features/animation.js").AnimationInterface;
} & typeof ModelViewerElementBase;
export declare type ModelViewerElement = InstanceType<typeof ModelViewerElement>;
export { RGB, RGBA } from './three-components/gltf-instance/gltf-2.0';
declare global {
    interface HTMLElementTagNameMap {
        'model-viewer': ModelViewerElement;
    }
}
