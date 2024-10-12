/* @license
 * Copyright 2019 Google LLC. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var _a, _b, _c, _d, _e, _f, _g, _h, _j, _k, _l, _m, _o;
import { ReactiveElement } from 'lit';
import { property } from 'lit/decorators.js';
import { HAS_INTERSECTION_OBSERVER, HAS_RESIZE_OBSERVER } from './constants.js';
import { $updateEnvironment } from './features/environment.js';
import { makeTemplate } from './template.js';
import { $evictionPolicy, CachingGLTFLoader } from './three-components/CachingGLTFLoader.js';
import { ModelScene } from './three-components/ModelScene.js';
import { Renderer } from './three-components/Renderer.js';
import { clamp, debounce } from './utilities.js';
import { dataUrlToBlob } from './utilities/data-conversion.js';
import { ProgressTracker } from './utilities/progress-tracker.js';
const CLEAR_MODEL_TIMEOUT_MS = 1000;
const FALLBACK_SIZE_UPDATE_THRESHOLD_MS = 50;
const ANNOUNCE_MODEL_VISIBILITY_DEBOUNCE_THRESHOLD = 0;
const UNSIZED_MEDIA_WIDTH = 300;
const UNSIZED_MEDIA_HEIGHT = 150;
export const blobCanvas = document.createElement('canvas');
const $fallbackResizeHandler = Symbol('fallbackResizeHandler');
const $defaultAriaLabel = Symbol('defaultAriaLabel');
const $resizeObserver = Symbol('resizeObserver');
const $clearModelTimeout = Symbol('clearModelTimeout');
const $onContextLost = Symbol('onContextLost');
const $loaded = Symbol('loaded');
const $status = Symbol('status');
const $onFocus = Symbol('onFocus');
const $onBlur = Symbol('onBlur');
export const $updateSize = Symbol('updateSize');
export const $intersectionObserver = Symbol('intersectionObserver');
export const $isElementInViewport = Symbol('isElementInViewport');
export const $announceModelVisibility = Symbol('announceModelVisibility');
export const $ariaLabel = Symbol('ariaLabel');
export const $altDefaulted = Symbol('altDefaulted');
export const $statusElement = Symbol('statusElement');
export const $updateStatus = Symbol('updateStatus');
export const $loadedTime = Symbol('loadedTime');
export const $updateSource = Symbol('updateSource');
export const $markLoaded = Symbol('markLoaded');
export const $container = Symbol('container');
export const $userInputElement = Symbol('input');
export const $canvas = Symbol('canvas');
export const $scene = Symbol('scene');
export const $needsRender = Symbol('needsRender');
export const $tick = Symbol('tick');
export const $onModelLoad = Symbol('onModelLoad');
export const $onResize = Symbol('onResize');
export const $renderer = Symbol('renderer');
export const $progressTracker = Symbol('progressTracker');
export const $getLoaded = Symbol('getLoaded');
export const $getModelIsVisible = Symbol('getModelIsVisible');
export const $shouldAttemptPreload = Symbol('shouldAttemptPreload');
export const toVector3D = (v) => {
    return {
        x: v.x,
        y: v.y,
        z: v.z,
        toString() {
            return `${this.x}m ${this.y}m ${this.z}m`;
        }
    };
};
export const toVector2D = (v) => {
    return {
        u: v.x,
        v: v.y,
        toString() {
            return `${this.u} ${this.v}`;
        }
    };
};
/**
 * Definition for a basic <model-viewer> element.
 */
export default class ModelViewerElementBase extends ReactiveElement {
    /**
     * Creates a new ModelViewerElement.
     */
    constructor() {
        super();
        this.alt = null;
        this.src = null;
        this.withCredentials = false;
        /**
         * Generates a 3D model schema https://schema.org/3DModel associated with
         * the loaded src and inserts it into the header of the page for search
         * engines to crawl.
         */
        this.generateSchema = false;
        this[_a] = false;
        this[_b] = false;
        this[_c] = 0;
        this[_d] = '';
        this[_e] = null;
        this[_f] = debounce(() => {
            const boundingRect = this.getBoundingClientRect();
            this[$updateSize](boundingRect);
        }, FALLBACK_SIZE_UPDATE_THRESHOLD_MS);
        this[_g] = debounce((oldVisibility) => {
            const newVisibility = this.modelIsVisible;
            if (newVisibility !== oldVisibility) {
                this.dispatchEvent(new CustomEvent('model-visibility', { detail: { visible: newVisibility } }));
            }
        }, ANNOUNCE_MODEL_VISIBILITY_DEBOUNCE_THRESHOLD);
        this[_h] = null;
        this[_j] = null;
        this[_k] = new ProgressTracker();
        this[_l] = () => {
            this[$statusElement].textContent = this[$status];
        };
        this[_m] = () => {
            this[$statusElement].textContent = '';
        };
        this[_o] = (event) => {
            this.dispatchEvent(new CustomEvent('error', { detail: { type: 'webglcontextlost', sourceError: event.sourceEvent } }));
        };
        this.attachShadow({ mode: 'open' });
        const shadowRoot = this.shadowRoot;
        makeTemplate(shadowRoot);
        this[$container] = shadowRoot.querySelector('.container');
        this[$userInputElement] =
            shadowRoot.querySelector('.userInput');
        this[$canvas] = shadowRoot.querySelector('canvas');
        this[$statusElement] =
            shadowRoot.querySelector('#status');
        this[$defaultAriaLabel] =
            this[$userInputElement].getAttribute('aria-label');
        // Because of potential race conditions related to invoking the constructor
        // we only use the bounding rect to set the initial size if the element is
        // already connected to the document:
        let width, height;
        if (this.isConnected) {
            const rect = this.getBoundingClientRect();
            width = rect.width;
            height = rect.height;
        }
        else {
            width = UNSIZED_MEDIA_WIDTH;
            height = UNSIZED_MEDIA_HEIGHT;
        }
        // Create the underlying ModelScene.
        this[$scene] =
            new ModelScene({ canvas: this[$canvas], element: this, width, height });
        // Update initial size on microtask timing so that subclasses have a
        // chance to initialize
        Promise.resolve().then(() => {
            this[$updateSize](this.getBoundingClientRect());
        });
        if (HAS_RESIZE_OBSERVER) {
            // Set up a resize observer so we can scale our canvas
            // if our <model-viewer> changes
            this[$resizeObserver] =
                new ResizeObserver((entries) => {
                    // Don't resize anything if in AR mode; otherwise the canvas
                    // scaling to fullscreen on entering AR will clobber the flat/2d
                    // dimensions of the element.
                    if (this[$renderer].isPresenting) {
                        return;
                    }
                    for (let entry of entries) {
                        if (entry.target === this) {
                            this[$updateSize](entry.contentRect);
                        }
                    }
                });
        }
        if (HAS_INTERSECTION_OBSERVER) {
            this[$intersectionObserver] = new IntersectionObserver(entries => {
                for (let entry of entries) {
                    if (entry.target === this) {
                        const oldVisibility = this.modelIsVisible;
                        this[$isElementInViewport] = entry.isIntersecting;
                        this[$announceModelVisibility](oldVisibility);
                        if (this[$isElementInViewport] && !this.loaded) {
                            this[$updateSource]();
                        }
                    }
                }
            }, {
                root: null,
                // We used to have margin here, but it was causing animated models below
                // the fold to steal the frame budget. Weirder still, it would also
                // cause input events to be swallowed, sometimes for seconds on the
                // model above the fold, but only when the animated model was completely
                // below. Setting this margin to zero fixed it.
                rootMargin: '0px',
                // With zero threshold, an element adjacent to but not intersecting the
                // viewport will be reported as intersecting, which will cause
                // unnecessary rendering. Any slight positive threshold alleviates this.
                threshold: 0.00001,
            });
        }
        else {
            // If there is no intersection observer, then all models should be visible
            // at all times:
            this[$isElementInViewport] = true;
        }
    }
    static get is() {
        return 'model-viewer';
    }
    /** @export */
    static set modelCacheSize(value) {
        CachingGLTFLoader[$evictionPolicy].evictionThreshold = value;
    }
    /** @export */
    static get modelCacheSize() {
        return CachingGLTFLoader[$evictionPolicy].evictionThreshold;
    }
    /** @export */
    static set minimumRenderScale(value) {
        if (value > 1) {
            console.warn('<model-viewer> minimumRenderScale has been clamped to a maximum value of 1.');
        }
        if (value <= 0) {
            console.warn('<model-viewer> minimumRenderScale has been clamped to a minimum value of 0.25.');
        }
        Renderer.singleton.minScale = value;
    }
    /** @export */
    static get minimumRenderScale() {
        return Renderer.singleton.minScale;
    }
    /** @export */
    get loaded() {
        return this[$getLoaded]();
    }
    get [(_a = $isElementInViewport, _b = $loaded, _c = $loadedTime, _d = $status, _e = $clearModelTimeout, _f = $fallbackResizeHandler, _g = $announceModelVisibility, _h = $resizeObserver, _j = $intersectionObserver, _k = $progressTracker, $renderer)]() {
        return Renderer.singleton;
    }
    /** @export */
    get modelIsVisible() {
        return this[$getModelIsVisible]();
    }
    connectedCallback() {
        super.connectedCallback && super.connectedCallback();
        if (HAS_RESIZE_OBSERVER) {
            this[$resizeObserver].observe(this);
        }
        else {
            self.addEventListener('resize', this[$fallbackResizeHandler]);
        }
        if (HAS_INTERSECTION_OBSERVER) {
            this[$intersectionObserver].observe(this);
        }
        this.addEventListener('focus', this[$onFocus]);
        this.addEventListener('blur', this[$onBlur]);
        const renderer = this[$renderer];
        renderer.addEventListener('contextlost', this[$onContextLost]);
        renderer.registerScene(this[$scene]);
        if (this[$clearModelTimeout] != null) {
            self.clearTimeout(this[$clearModelTimeout]);
            this[$clearModelTimeout] = null;
            // Force an update in case the model has been evicted from our GLTF cache
            // @see https://lit-element.polymer-project.org/guide/lifecycle#requestupdate
            this.requestUpdate('src', null);
        }
    }
    disconnectedCallback() {
        super.disconnectedCallback && super.disconnectedCallback();
        if (HAS_RESIZE_OBSERVER) {
            this[$resizeObserver].unobserve(this);
        }
        else {
            self.removeEventListener('resize', this[$fallbackResizeHandler]);
        }
        if (HAS_INTERSECTION_OBSERVER) {
            this[$intersectionObserver].unobserve(this);
        }
        this.removeEventListener('focus', this[$onFocus]);
        this.removeEventListener('blur', this[$onBlur]);
        const renderer = this[$renderer];
        renderer.removeEventListener('contextlost', this[$onContextLost]);
        renderer.unregisterScene(this[$scene]);
        this[$clearModelTimeout] = self.setTimeout(() => {
            this[$scene].dispose();
            this[$clearModelTimeout] = null;
        }, CLEAR_MODEL_TIMEOUT_MS);
    }
    updated(changedProperties) {
        super.updated(changedProperties);
        // NOTE(cdata): If a property changes from values A -> B -> A in the space
        // of a microtask, LitElement/UpdatingElement will notify of a change even
        // though the value has effectively not changed, so we need to check to make
        // sure that the value has actually changed before changing the loaded flag.
        if (changedProperties.has('src')) {
            if (this.src == null) {
                this[$loaded] = false;
                this[$loadedTime] = 0;
                this[$scene].reset();
            }
            else if (this.src !== this[$scene].url) {
                this[$loaded] = false;
                this[$loadedTime] = 0;
                this[$updateSource]();
            }
        }
        if (changedProperties.has('alt')) {
            this[$userInputElement].setAttribute('aria-label', this[$ariaLabel]);
        }
        if (changedProperties.has('withCredentials')) {
            CachingGLTFLoader.withCredentials = this.withCredentials;
        }
        if (changedProperties.has('generateSchema')) {
            if (this.generateSchema) {
                this[$scene].updateSchema(this.src);
            }
            else {
                this[$scene].updateSchema(null);
            }
        }
    }
    /** @export */
    toDataURL(type, encoderOptions) {
        return this[$renderer]
            .displayCanvas(this[$scene])
            .toDataURL(type, encoderOptions);
    }
    /** @export */
    async toBlob(options) {
        const mimeType = options ? options.mimeType : undefined;
        const qualityArgument = options ? options.qualityArgument : undefined;
        const useIdealAspect = options ? options.idealAspect : undefined;
        const { width, height, idealAspect, aspect } = this[$scene];
        const { dpr, scaleFactor } = this[$renderer];
        let outputWidth = width * scaleFactor * dpr;
        let outputHeight = height * scaleFactor * dpr;
        let offsetX = 0;
        let offsetY = 0;
        if (useIdealAspect === true) {
            if (idealAspect > aspect) {
                const oldHeight = outputHeight;
                outputHeight = Math.round(outputWidth / idealAspect);
                offsetY = (oldHeight - outputHeight) / 2;
            }
            else {
                const oldWidth = outputWidth;
                outputWidth = Math.round(outputHeight * idealAspect);
                offsetX = (oldWidth - outputWidth) / 2;
            }
        }
        blobCanvas.width = outputWidth;
        blobCanvas.height = outputHeight;
        try {
            return new Promise(async (resolve, reject) => {
                blobCanvas.getContext('2d').drawImage(this[$renderer].displayCanvas(this[$scene]), offsetX, offsetY, outputWidth, outputHeight, 0, 0, outputWidth, outputHeight);
                if (blobCanvas.msToBlob) {
                    // NOTE: msToBlob only returns image/png
                    // so ensure mimeType is not specified (defaults to image/png)
                    // or is image/png, otherwise fallback to using toDataURL on IE.
                    if (!mimeType || mimeType === 'image/png') {
                        return resolve(blobCanvas.msToBlob());
                    }
                }
                if (!blobCanvas.toBlob) {
                    return resolve(await dataUrlToBlob(blobCanvas.toDataURL(mimeType, qualityArgument)));
                }
                blobCanvas.toBlob((blob) => {
                    if (!blob) {
                        return reject(new Error('Unable to retrieve canvas blob'));
                    }
                    resolve(blob);
                }, mimeType, qualityArgument);
            });
        }
        finally {
            this[$updateSize]({ width, height });
        }
        ;
    }
    registerRenderer(renderer) {
        this[$scene].externalRenderer = renderer;
    }
    unregisterRenderer() {
        this[$scene].externalRenderer = null;
    }
    get [$ariaLabel]() {
        return this[$altDefaulted];
    }
    get [$altDefaulted]() {
        return (this.alt == null || this.alt === 'null') ? this[$defaultAriaLabel] :
            this.alt;
    }
    // NOTE(cdata): Although this may seem extremely redundant, it is required in
    // order to support overloading when TypeScript is compiled to ES5
    // @see https://github.com/Polymer/lit-element/pull/745
    // @see https://github.com/microsoft/TypeScript/issues/338
    [$getLoaded]() {
        return this[$loaded];
    }
    // @see [$getLoaded]
    [$getModelIsVisible]() {
        return this.loaded && this[$isElementInViewport];
    }
    [$shouldAttemptPreload]() {
        return !!this.src && this[$isElementInViewport];
    }
    /**
     * Called on initialization and when the resize observer fires.
     */
    [$updateSize]({ width, height }) {
        this[$container].style.width = `${width}px`;
        this[$container].style.height = `${height}px`;
        this[$onResize]({ width: parseFloat(width), height: parseFloat(height) });
    }
    [$tick](_time, _delta) {
    }
    [$markLoaded]() {
        if (this[$loaded]) {
            return;
        }
        this[$loaded] = true;
        this[$loadedTime] = performance.now();
    }
    [$needsRender]() {
        this[$scene].queueRender();
    }
    [$onModelLoad]() {
    }
    [$updateStatus](status) {
        this[$status] = status;
        const rootNode = this.getRootNode();
        // Only change the aria-label if <model-viewer> is currently focused:
        if (rootNode != null && rootNode.activeElement === this &&
            this[$statusElement].textContent != status) {
            this[$statusElement].textContent = status;
        }
    }
    [(_l = $onFocus, _m = $onBlur, $onResize)](e) {
        this[$scene].setSize(e.width, e.height);
    }
    /**
     * Parses the element for an appropriate source URL and
     * sets the views to use the new model based.
     */
    async [(_o = $onContextLost, $updateSource)]() {
        const scene = this[$scene];
        if (this.loaded || !this[$shouldAttemptPreload]() ||
            this.src === scene.url) {
            return;
        }
        if (this.generateSchema) {
            scene.updateSchema(this.src);
        }
        this[$updateStatus]('Loading');
        // If we are loading a new model, we need to stop the animation of
        // the current one (if any is playing). Otherwise, we might lose
        // the reference to the scene root and running actions start to
        // throw exceptions and/or behave in unexpected ways:
        scene.stopAnimation();
        const updateSourceProgress = this[$progressTracker].beginActivity();
        const source = this.src;
        try {
            const srcUpdated = scene.setSource(source, (progress) => updateSourceProgress(clamp(progress, 0, 1) * 0.95));
            const envUpdated = this[$updateEnvironment]();
            await Promise.all([srcUpdated, envUpdated]);
            this[$markLoaded]();
            this[$onModelLoad]();
            // Wait for shaders to compile and pixels to be drawn.
            await new Promise(resolve => {
                requestAnimationFrame(() => {
                    requestAnimationFrame(() => {
                        this.dispatchEvent(new CustomEvent('load', { detail: { url: source } }));
                        resolve();
                    });
                });
            });
        }
        catch (error) {
            this.dispatchEvent(new CustomEvent('error', { detail: { type: 'loadfailure', sourceError: error } }));
        }
        finally {
            updateSourceProgress(1.0);
        }
    }
}
__decorate([
    property({ type: String })
], ModelViewerElementBase.prototype, "alt", void 0);
__decorate([
    property({ type: String })
], ModelViewerElementBase.prototype, "src", void 0);
__decorate([
    property({ type: Boolean, attribute: 'with-credentials' })
], ModelViewerElementBase.prototype, "withCredentials", void 0);
__decorate([
    property({ type: Boolean, attribute: 'generate-schema' })
], ModelViewerElementBase.prototype, "generateSchema", void 0);
//# sourceMappingURL=model-viewer-base.js.map