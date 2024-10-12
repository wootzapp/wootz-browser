import { GLTF } from 'three/examples/jsm/loaders/GLTFLoader.js';
import { $clone, $prepare, GLTFInstance, PreparedGLTF } from '../GLTFInstance.js';
import { CorrelatedSceneGraph } from './correlated-scene-graph.js';
declare const $correlatedSceneGraph: unique symbol;
interface PreparedModelViewerGLTF extends PreparedGLTF {
    [$correlatedSceneGraph]?: CorrelatedSceneGraph;
}
/**
 * This specialization of GLTFInstance collects all of the processing needed
 * to prepare a model and to clone it making special considerations for
 * <model-viewer> use cases.
 */
export declare class ModelViewerGLTFInstance extends GLTFInstance {
    /**
     * @override
     */
    protected static [$prepare](source: GLTF): PreparedModelViewerGLTF;
    get correlatedSceneGraph(): CorrelatedSceneGraph;
    /**
     * @override
     */
    [$clone](): PreparedGLTF;
}
export {};
