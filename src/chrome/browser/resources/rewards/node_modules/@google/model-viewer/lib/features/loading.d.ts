import ModelViewerElementBase, { Vector3D } from '../model-viewer-base.js';
import { Constructor } from '../utilities.js';
export declare type RevealAttributeValue = 'auto' | 'manual';
export declare type LoadingAttributeValue = 'auto' | 'lazy' | 'eager';
export declare const POSTER_TRANSITION_TIME = 300;
export declare const PROGRESS_BAR_UPDATE_THRESHOLD = 100;
export declare const $defaultProgressBarElement: unique symbol;
export declare const $posterContainerElement: unique symbol;
export declare const $defaultPosterElement: unique symbol;
export declare interface LoadingInterface {
    poster: string | null;
    reveal: RevealAttributeValue;
    loading: LoadingAttributeValue;
    readonly loaded: boolean;
    readonly modelIsVisible: boolean;
    dismissPoster(): void;
    showPoster(): void;
    getDimensions(): Vector3D;
    getBoundingBoxCenter(): Vector3D;
}
export declare interface LoadingStaticInterface {
    dracoDecoderLocation: string;
    ktx2TranscoderLocation: string;
    meshoptDecoderLocation: string;
    mapURLs(callback: (url: string) => string): void;
}
export interface ModelViewerGlobalConfig {
    dracoDecoderLocation?: string;
    ktx2TranscoderLocation?: string;
    meshoptDecoderLocation?: string;
    powerPreference?: string;
}
/**
 * LoadingMixin implements features related to lazy loading, as well as
 * presentation details related to the pre-load / pre-render presentation of a
 * <model-viewer>
 *
 * This mixin implements support for models with DRACO-compressed meshes.
 * The DRACO decoder will be loaded on-demand if a glTF that uses the DRACO mesh
 * compression extension is encountered.
 *
 * By default, the DRACO decoder will be loaded from a Google CDN. It is
 * possible to customize where the decoder is loaded from by defining a global
 * configuration option for `<model-viewer>` like so:
 *
 * ```html
 * <script>
 * self.ModelViewerElement = self.ModelViewerElement || {};
 * self.ModelViewerElement.dracoDecoderLocation =
 *     'http://example.com/location/of/draco/decoder/files/';
 * </script>
 * ```
 *
 * Note that the above configuration strategy must be performed *before* the
 * first `<model-viewer>` element is created in the browser. The configuration
 * can be done anywhere, but the easiest way to ensure it is done at the right
 * time is to do it in the `<head>` of the HTML document. This is the
 * recommended way to set the location because it is most compatible with
 * scenarios where the `<model-viewer>` library is lazily loaded.
 *
 * If you absolutely have to set the DRACO decoder location *after* the first
 * `<model-viewer>` element is created, you can do it this way:
 *
 * ```html
 * <script>
 * const ModelViewerElement = customElements.get('model-viewer');
 * ModelViewerElement.dracoDecoderLocation =
 *     'http://example.com/location/of/draco/decoder/files/';
 * </script>
 * ```
 *
 * Note that the above configuration approach will not work until *after*
 * `<model-viewer>` is defined in the browser. Also note that this configuration
 * *must* be set *before* the first DRACO model is fully loaded.
 *
 * It is recommended that users who intend to take advantage of DRACO mesh
 * compression consider whether or not it is acceptable for their use case to
 * have code side-loaded from a Google CDN. If it is not acceptable, then the
 * location must be customized before loading any DRACO models in order to cause
 * the decoder to be loaded from an alternative, acceptable location.
 */
export declare const LoadingMixin: <T extends Constructor<ModelViewerElementBase, object>>(ModelViewerElement: T) => {
    new (...args: any[]): LoadingInterface;
    prototype: LoadingInterface;
} & LoadingStaticInterface & T;
