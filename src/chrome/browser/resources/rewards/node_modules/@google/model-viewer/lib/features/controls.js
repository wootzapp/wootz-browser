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
import { property } from 'lit/decorators.js';
import { Spherical, Vector3 } from 'three';
import { style } from '../decorators.js';
import { $ariaLabel, $container, $getModelIsVisible, $loadedTime, $needsRender, $onModelLoad, $onResize, $renderer, $scene, $tick, $updateStatus, $userInputElement, toVector3D } from '../model-viewer-base.js';
import { degreesToRadians, normalizeUnit } from '../styles/conversions.js';
import { StyleEvaluator } from '../styles/evaluators.js';
import { numberNode, parseExpressions } from '../styles/parsers.js';
import { DECAY_MILLISECONDS } from '../three-components/Damper.js';
import { ChangeSource, SmoothControls } from '../three-components/SmoothControls.js';
import { timeline } from '../utilities/animation.js';
// NOTE(cdata): The following "animation" timing functions are deliberately
// being used in favor of CSS animations. In Safari 12.1 and 13, CSS animations
// would cause the interaction prompt to glitch unexpectedly
// @see https://github.com/google/model-viewer/issues/839
const PROMPT_ANIMATION_TIME = 5000;
// For timing purposes, a "frame" is a timing agnostic relative unit of time
// and a "value" is a target value for the Frame.
const wiggle = timeline({
    initialValue: 0,
    keyframes: [
        { frames: 5, value: -1 },
        { frames: 1, value: -1 },
        { frames: 8, value: 1 },
        { frames: 1, value: 1 },
        { frames: 5, value: 0 },
        { frames: 18, value: 0 }
    ]
});
const fade = timeline({
    initialValue: 0,
    keyframes: [
        { frames: 1, value: 1 },
        { frames: 5, value: 1 },
        { frames: 1, value: 0 },
        { frames: 6, value: 0 }
    ]
});
export const DEFAULT_FOV_DEG = 30;
export const DEFAULT_MIN_FOV_DEG = 12;
export const DEFAULT_CAMERA_ORBIT = '0deg 75deg 105%';
const DEFAULT_CAMERA_TARGET = 'auto auto auto';
const DEFAULT_FIELD_OF_VIEW = 'auto';
const MINIMUM_RADIUS_RATIO = 2.2;
const AZIMUTHAL_QUADRANT_LABELS = ['front', 'right', 'back', 'left'];
const POLAR_TRIENT_LABELS = ['upper-', '', 'lower-'];
export const DEFAULT_INTERACTION_PROMPT_THRESHOLD = 3000;
export const INTERACTION_PROMPT = '. Use mouse, touch or arrow keys to move.';
export const InteractionPromptStrategy = {
    AUTO: 'auto',
    NONE: 'none'
};
export const InteractionPromptStyle = {
    BASIC: 'basic',
    WIGGLE: 'wiggle'
};
export const TouchAction = {
    PAN_Y: 'pan-y',
    PAN_X: 'pan-x',
    NONE: 'none'
};
export const fieldOfViewIntrinsics = () => {
    return {
        basis: [degreesToRadians(numberNode(DEFAULT_FOV_DEG, 'deg'))],
        keywords: { auto: [null] }
    };
};
const minFieldOfViewIntrinsics = () => {
    return {
        basis: [degreesToRadians(numberNode(DEFAULT_MIN_FOV_DEG, 'deg'))],
        keywords: { auto: [null] }
    };
};
export const cameraOrbitIntrinsics = (() => {
    const defaultTerms = parseExpressions(DEFAULT_CAMERA_ORBIT)[0]
        .terms;
    const theta = normalizeUnit(defaultTerms[0]);
    const phi = normalizeUnit(defaultTerms[1]);
    return (element) => {
        const radius = element[$scene].idealCameraDistance();
        return {
            basis: [theta, phi, numberNode(radius, 'm')],
            keywords: { auto: [null, null, numberNode(105, '%')] }
        };
    };
})();
const minCameraOrbitIntrinsics = (element) => {
    const radius = MINIMUM_RADIUS_RATIO * element[$scene].boundingSphere.radius;
    return {
        basis: [
            numberNode(-Infinity, 'rad'),
            numberNode(Math.PI / 8, 'rad'),
            numberNode(radius, 'm')
        ],
        keywords: { auto: [null, null, null] }
    };
};
const maxCameraOrbitIntrinsics = (element) => {
    const orbitIntrinsics = cameraOrbitIntrinsics(element);
    const evaluator = new StyleEvaluator([], orbitIntrinsics);
    const defaultRadius = evaluator.evaluate()[2];
    return {
        basis: [
            numberNode(Infinity, 'rad'),
            numberNode(Math.PI - Math.PI / 8, 'rad'),
            numberNode(defaultRadius, 'm')
        ],
        keywords: { auto: [null, null, null] }
    };
};
export const cameraTargetIntrinsics = (element) => {
    const center = element[$scene].boundingBox.getCenter(new Vector3());
    return {
        basis: [
            numberNode(center.x, 'm'),
            numberNode(center.y, 'm'),
            numberNode(center.z, 'm')
        ],
        keywords: { auto: [null, null, null] }
    };
};
const HALF_PI = Math.PI / 2.0;
const THIRD_PI = Math.PI / 3.0;
const QUARTER_PI = HALF_PI / 2.0;
const TAU = 2.0 * Math.PI;
export const $controls = Symbol('controls');
export const $panElement = Symbol('panElement');
export const $promptElement = Symbol('promptElement');
export const $promptAnimatedContainer = Symbol('promptAnimatedContainer');
export const $fingerAnimatedContainers = Symbol('fingerAnimatedContainers');
const $deferInteractionPrompt = Symbol('deferInteractionPrompt');
const $updateAria = Symbol('updateAria');
const $updateCameraForRadius = Symbol('updateCameraForRadius');
const $onChange = Symbol('onChange');
const $onPointerChange = Symbol('onPointerChange');
const $waitingToPromptUser = Symbol('waitingToPromptUser');
const $userHasInteracted = Symbol('userHasInteracted');
const $promptElementVisibleTime = Symbol('promptElementVisibleTime');
const $lastPromptOffset = Symbol('lastPromptOffset');
const $lastSpherical = Symbol('lastSpherical');
const $jumpCamera = Symbol('jumpCamera');
const $initialized = Symbol('initialized');
const $maintainThetaPhi = Symbol('maintainThetaPhi');
const $syncCameraOrbit = Symbol('syncCameraOrbit');
const $syncFieldOfView = Symbol('syncFieldOfView');
const $syncCameraTarget = Symbol('syncCameraTarget');
const $syncMinCameraOrbit = Symbol('syncMinCameraOrbit');
const $syncMaxCameraOrbit = Symbol('syncMaxCameraOrbit');
const $syncMinFieldOfView = Symbol('syncMinFieldOfView');
const $syncMaxFieldOfView = Symbol('syncMaxFieldOfView');
export const ControlsMixin = (ModelViewerElement) => {
    var _a, _b, _c, _d, _e, _f, _g, _h, _j, _k, _l, _m, _o, _p, _q;
    class ControlsModelViewerElement extends ModelViewerElement {
        constructor() {
            super(...arguments);
            this.cameraControls = false;
            this.cameraOrbit = DEFAULT_CAMERA_ORBIT;
            this.cameraTarget = DEFAULT_CAMERA_TARGET;
            this.fieldOfView = DEFAULT_FIELD_OF_VIEW;
            this.minCameraOrbit = 'auto';
            this.maxCameraOrbit = 'auto';
            this.minFieldOfView = 'auto';
            this.maxFieldOfView = 'auto';
            this.interactionPromptThreshold = DEFAULT_INTERACTION_PROMPT_THRESHOLD;
            this.interactionPrompt = InteractionPromptStrategy.AUTO;
            this.interactionPromptStyle = InteractionPromptStyle.WIGGLE;
            this.orbitSensitivity = 1;
            this.touchAction = TouchAction.NONE;
            this.disableZoom = false;
            this.disablePan = false;
            this.disableTap = false;
            this.interpolationDecay = DECAY_MILLISECONDS;
            this[_a] = this.shadowRoot.querySelector('.interaction-prompt');
            this[_b] = this.shadowRoot.querySelector('#prompt');
            this[_c] = [
                this.shadowRoot.querySelector('#finger0'),
                this.shadowRoot.querySelector('#finger1')
            ];
            this[_d] = this.shadowRoot.querySelector('.pan-target');
            this[_e] = 0;
            this[_f] = Infinity;
            this[_g] = false;
            this[_h] = false;
            this[_j] = new SmoothControls(this[$scene].camera, this[$userInputElement], this[$scene]);
            this[_k] = new Spherical();
            this[_l] = false;
            this[_m] = false;
            this[_o] = false;
            this[_p] = ({ source }) => {
                this[$updateAria]();
                this[$needsRender]();
                if (source === ChangeSource.USER_INTERACTION) {
                    this[$userHasInteracted] = true;
                    this[$deferInteractionPrompt]();
                }
                this.dispatchEvent(new CustomEvent('camera-change', { detail: { source } }));
            };
            this[_q] = (event) => {
                if (event.type === 'pointer-change-start') {
                    this[$container].classList.add('pointer-tumbling');
                }
                else {
                    this[$container].classList.remove('pointer-tumbling');
                }
            };
        }
        get inputSensitivity() {
            return this[$controls].inputSensitivity;
        }
        set inputSensitivity(value) {
            this[$controls].inputSensitivity = value;
        }
        getCameraOrbit() {
            const { theta, phi, radius } = this[$lastSpherical];
            return {
                theta,
                phi,
                radius,
                toString() {
                    return `${this.theta}rad ${this.phi}rad ${this.radius}m`;
                }
            };
        }
        getCameraTarget() {
            return toVector3D(this[$renderer].isPresenting ? this[$renderer].arRenderer.target :
                this[$scene].getTarget());
        }
        getFieldOfView() {
            return this[$controls].getFieldOfView();
        }
        // Provided so user code does not have to parse these from attributes.
        getMinimumFieldOfView() {
            return this[$controls].options.minimumFieldOfView;
        }
        getMaximumFieldOfView() {
            return this[$controls].options.maximumFieldOfView;
        }
        getIdealAspect() {
            return this[$scene].idealAspect;
        }
        jumpCameraToGoal() {
            this[$jumpCamera] = true;
            this.requestUpdate($jumpCamera, false);
        }
        resetInteractionPrompt() {
            this[$lastPromptOffset] = 0;
            this[$promptElementVisibleTime] = Infinity;
            this[$userHasInteracted] = false;
            this[$waitingToPromptUser] =
                this.interactionPrompt === InteractionPromptStrategy.AUTO &&
                    this.cameraControls;
        }
        zoom(keyPresses) {
            const event = new WheelEvent('wheel', { deltaY: -30 * keyPresses });
            this[$userInputElement].dispatchEvent(event);
        }
        connectedCallback() {
            super.connectedCallback();
            this[$controls].addEventListener('change', this[$onChange]);
            this[$controls].addEventListener('pointer-change-start', this[$onPointerChange]);
            this[$controls].addEventListener('pointer-change-end', this[$onPointerChange]);
        }
        disconnectedCallback() {
            super.disconnectedCallback();
            this[$controls].removeEventListener('change', this[$onChange]);
            this[$controls].removeEventListener('pointer-change-start', this[$onPointerChange]);
            this[$controls].removeEventListener('pointer-change-end', this[$onPointerChange]);
        }
        updated(changedProperties) {
            super.updated(changedProperties);
            const controls = this[$controls];
            const scene = this[$scene];
            if (changedProperties.has('cameraControls')) {
                if (this.cameraControls) {
                    controls.enableInteraction();
                    if (this.interactionPrompt === InteractionPromptStrategy.AUTO) {
                        this[$waitingToPromptUser] = true;
                    }
                }
                else {
                    controls.disableInteraction();
                    this[$deferInteractionPrompt]();
                }
                this[$userInputElement].setAttribute('aria-label', this[$ariaLabel]);
            }
            if (changedProperties.has('disableZoom')) {
                controls.disableZoom = this.disableZoom;
            }
            if (changedProperties.has('disablePan')) {
                controls.enablePan = !this.disablePan;
            }
            if (changedProperties.has('disableTap')) {
                controls.enableTap = !this.disableTap;
            }
            if (changedProperties.has('interactionPrompt') ||
                changedProperties.has('cameraControls') ||
                changedProperties.has('src')) {
                if (this.interactionPrompt === InteractionPromptStrategy.AUTO &&
                    this.cameraControls && !this[$userHasInteracted]) {
                    this[$waitingToPromptUser] = true;
                }
                else {
                    this[$deferInteractionPrompt]();
                }
            }
            if (changedProperties.has('interactionPromptStyle')) {
                this[$promptAnimatedContainer].style.opacity =
                    this.interactionPromptStyle == InteractionPromptStyle.BASIC ? '1' :
                        '0';
            }
            if (changedProperties.has('touchAction')) {
                const touchAction = this.touchAction;
                controls.applyOptions({ touchAction });
                controls.updateTouchActionStyle();
            }
            if (changedProperties.has('orbitSensitivity')) {
                controls.orbitSensitivity = this.orbitSensitivity;
            }
            if (changedProperties.has('interpolationDecay')) {
                controls.setDamperDecayTime(this.interpolationDecay);
                scene.setTargetDamperDecayTime(this.interpolationDecay);
            }
            if (this[$jumpCamera] === true) {
                Promise.resolve().then(() => {
                    controls.jumpToGoal();
                    scene.jumpToGoal();
                    this[$jumpCamera] = false;
                });
            }
        }
        async updateFraming() {
            const scene = this[$scene];
            const oldFramedFoV = scene.adjustedFoV(scene.framedFoVDeg);
            await scene.updateFraming();
            const newFramedFoV = scene.adjustedFoV(scene.framedFoVDeg);
            const zoom = this[$controls].getFieldOfView() / oldFramedFoV;
            this[$controls].setFieldOfView(newFramedFoV * zoom);
            this[$maintainThetaPhi] = true;
            this.requestUpdate('maxFieldOfView');
            this.requestUpdate('fieldOfView');
            this.requestUpdate('minCameraOrbit');
            this.requestUpdate('maxCameraOrbit');
            this.requestUpdate('cameraOrbit');
            await this.updateComplete;
        }
        interact(duration, finger0, finger1) {
            const inputElement = this[$userInputElement];
            const fingerElements = this[$fingerAnimatedContainers];
            if (fingerElements[0].style.opacity === '1') {
                console.warn('interact() failed because an existing interaction is running.');
                return;
            }
            const xy = new Array();
            xy.push({ x: timeline(finger0.x), y: timeline(finger0.y) });
            const positions = [{ x: xy[0].x(0), y: xy[0].y(0) }];
            if (finger1 != null) {
                xy.push({ x: timeline(finger1.x), y: timeline(finger1.y) });
                positions.push({ x: xy[1].x(0), y: xy[1].y(0) });
            }
            let startTime = performance.now();
            const { width, height } = this[$scene];
            const dispatchTouches = (type) => {
                for (const [i, position] of positions.entries()) {
                    const { style } = fingerElements[i];
                    style.transform = `translateX(${width * position.x}px) translateY(${height * position.y}px)`;
                    if (type === 'pointerdown') {
                        style.opacity = '1';
                    }
                    else if (type === 'pointerup') {
                        style.opacity = '0';
                    }
                    const init = {
                        pointerId: i - 5678,
                        pointerType: 'touch',
                        target: inputElement,
                        clientX: width * position.x,
                        clientY: height * position.y,
                        altKey: true // flag that this is not a user interaction
                    };
                    inputElement.dispatchEvent(new PointerEvent(type, init));
                }
            };
            const moveTouches = () => {
                // Cancel interaction if something else moves the camera or input is
                // removed from the DOM.
                const { changeSource } = this[$controls];
                if (changeSource !== ChangeSource.AUTOMATIC ||
                    !inputElement.isConnected) {
                    for (const fingerElement of this[$fingerAnimatedContainers]) {
                        fingerElement.style.opacity = '0';
                    }
                    dispatchTouches('pointercancel');
                    this.dispatchEvent(new CustomEvent('interact-stopped', { detail: { source: changeSource } }));
                    document.removeEventListener('visibilitychange', onVisibilityChange);
                    return;
                }
                const time = Math.min(1, (performance.now() - startTime) / duration);
                for (const [i, position] of positions.entries()) {
                    position.x = xy[i].x(time);
                    position.y = xy[i].y(time);
                }
                dispatchTouches('pointermove');
                if (time < 1) {
                    requestAnimationFrame(moveTouches);
                }
                else {
                    dispatchTouches('pointerup');
                    this.dispatchEvent(new CustomEvent('interact-stopped', { detail: { source: changeSource } }));
                    document.removeEventListener('visibilitychange', onVisibilityChange);
                }
            };
            const onVisibilityChange = () => {
                let elapsed = 0;
                if (document.visibilityState === 'hidden') {
                    elapsed = performance.now() - startTime;
                }
                else {
                    startTime = performance.now() - elapsed;
                }
            };
            document.addEventListener('visibilitychange', onVisibilityChange);
            dispatchTouches('pointerdown');
            requestAnimationFrame(moveTouches);
        }
        [(_a = $promptElement, _b = $promptAnimatedContainer, _c = $fingerAnimatedContainers, _d = $panElement, _e = $lastPromptOffset, _f = $promptElementVisibleTime, _g = $userHasInteracted, _h = $waitingToPromptUser, _j = $controls, _k = $lastSpherical, _l = $jumpCamera, _m = $initialized, _o = $maintainThetaPhi, $syncFieldOfView)](style) {
            const scene = this[$scene];
            scene.framedFoVDeg = style[0] * 180 / Math.PI;
            this[$controls].setFieldOfView(scene.adjustedFoV(scene.framedFoVDeg));
        }
        [$syncCameraOrbit](style) {
            const controls = this[$controls];
            if (this[$maintainThetaPhi]) {
                const { theta, phi } = this.getCameraOrbit();
                style[0] = theta;
                style[1] = phi;
                this[$maintainThetaPhi] = false;
            }
            controls.changeSource = ChangeSource.NONE;
            controls.setOrbit(style[0], style[1], style[2]);
        }
        [$syncMinCameraOrbit](style) {
            this[$controls].applyOptions({
                minimumAzimuthalAngle: style[0],
                minimumPolarAngle: style[1],
                minimumRadius: style[2]
            });
            this.jumpCameraToGoal();
        }
        [$syncMaxCameraOrbit](style) {
            this[$controls].applyOptions({
                maximumAzimuthalAngle: style[0],
                maximumPolarAngle: style[1],
                maximumRadius: style[2]
            });
            this[$updateCameraForRadius](style[2]);
            this.jumpCameraToGoal();
        }
        [$syncMinFieldOfView](style) {
            this[$controls].applyOptions({ minimumFieldOfView: style[0] * 180 / Math.PI });
            this.jumpCameraToGoal();
        }
        [$syncMaxFieldOfView](style) {
            const fov = this[$scene].adjustedFoV(style[0] * 180 / Math.PI);
            this[$controls].applyOptions({ maximumFieldOfView: fov });
            this.jumpCameraToGoal();
        }
        [$syncCameraTarget](style) {
            const [x, y, z] = style;
            if (!this[$renderer].arRenderer.isPresenting) {
                this[$scene].setTarget(x, y, z);
            }
            this[$controls].changeSource = ChangeSource.NONE;
            this[$renderer].arRenderer.updateTarget();
        }
        [$tick](time, delta) {
            super[$tick](time, delta);
            if (this[$renderer].isPresenting || !this[$getModelIsVisible]()) {
                return;
            }
            const controls = this[$controls];
            const scene = this[$scene];
            const now = performance.now();
            if (this[$waitingToPromptUser]) {
                if (this.loaded &&
                    now > this[$loadedTime] + this.interactionPromptThreshold) {
                    this[$waitingToPromptUser] = false;
                    this[$promptElementVisibleTime] = now;
                    this[$promptElement].classList.add('visible');
                }
            }
            if (isFinite(this[$promptElementVisibleTime]) &&
                this.interactionPromptStyle === InteractionPromptStyle.WIGGLE) {
                const animationTime = ((now - this[$promptElementVisibleTime]) / PROMPT_ANIMATION_TIME) %
                    1;
                const offset = wiggle(animationTime);
                const opacity = fade(animationTime);
                this[$promptAnimatedContainer].style.opacity = `${opacity}`;
                if (offset !== this[$lastPromptOffset]) {
                    const xOffset = offset * scene.width * 0.05;
                    const deltaTheta = (offset - this[$lastPromptOffset]) * Math.PI / 16;
                    this[$promptAnimatedContainer].style.transform =
                        `translateX(${xOffset}px)`;
                    controls.changeSource = ChangeSource.AUTOMATIC;
                    controls.adjustOrbit(deltaTheta, 0, 0);
                    this[$lastPromptOffset] = offset;
                }
            }
            controls.update(time, delta);
            if (scene.updateTarget(delta)) {
                this[$onChange]({ type: 'change', source: controls.changeSource });
            }
        }
        [$deferInteractionPrompt]() {
            // Effectively cancel the timer waiting for user interaction:
            this[$waitingToPromptUser] = false;
            this[$promptElement].classList.remove('visible');
            this[$promptElementVisibleTime] = Infinity;
        }
        /**
         * Updates the camera's near and far planes to enclose the scene when
         * orbiting at the supplied radius.
         */
        [$updateCameraForRadius](radius) {
            const maximumRadius = Math.max(this[$scene].boundingSphere.radius, radius);
            const near = 0;
            const far = 2 * maximumRadius;
            this[$controls].updateNearFar(near, far);
        }
        [$updateAria]() {
            const { theta, phi } = this[$controls].getCameraSpherical(this[$lastSpherical]);
            const azimuthalQuadrant = (4 + Math.floor(((theta % TAU) + QUARTER_PI) / HALF_PI)) % 4;
            const polarTrient = Math.floor(phi / THIRD_PI);
            const azimuthalQuadrantLabel = AZIMUTHAL_QUADRANT_LABELS[azimuthalQuadrant];
            const polarTrientLabel = POLAR_TRIENT_LABELS[polarTrient];
            this[$updateStatus](`View from stage ${polarTrientLabel}${azimuthalQuadrantLabel}`);
        }
        get [$ariaLabel]() {
            return super[$ariaLabel] +
                (this.cameraControls ? INTERACTION_PROMPT : '');
        }
        async [$onResize](event) {
            const controls = this[$controls];
            const scene = this[$scene];
            const oldFramedFoV = scene.adjustedFoV(scene.framedFoVDeg);
            // The super of $onResize may update the scene's adjustedFoV, so we
            // compare the before and after to calculate the proper zoom.
            super[$onResize](event);
            const fovRatio = scene.adjustedFoV(scene.framedFoVDeg) / oldFramedFoV;
            const fov = controls.getFieldOfView() * (isFinite(fovRatio) ? fovRatio : 1);
            controls.updateAspect(this[$scene].aspect);
            this.requestUpdate('maxFieldOfView', this.maxFieldOfView);
            await this.updateComplete;
            this[$controls].setFieldOfView(fov);
            this.jumpCameraToGoal();
        }
        [$onModelLoad]() {
            super[$onModelLoad]();
            if (this[$initialized]) {
                this[$maintainThetaPhi] = true;
            }
            else {
                this[$initialized] = true;
            }
            this.requestUpdate('maxFieldOfView', this.maxFieldOfView);
            this.requestUpdate('fieldOfView', this.fieldOfView);
            this.requestUpdate('minCameraOrbit', this.minCameraOrbit);
            this.requestUpdate('maxCameraOrbit', this.maxCameraOrbit);
            this.requestUpdate('cameraOrbit', this.cameraOrbit);
            this.requestUpdate('cameraTarget', this.cameraTarget);
            this.jumpCameraToGoal();
        }
    }
    _p = $onChange, _q = $onPointerChange;
    __decorate([
        property({ type: Boolean, attribute: 'camera-controls' })
    ], ControlsModelViewerElement.prototype, "cameraControls", void 0);
    __decorate([
        style({
            intrinsics: cameraOrbitIntrinsics,
            observeEffects: true,
            updateHandler: $syncCameraOrbit
        }),
        property({ type: String, attribute: 'camera-orbit', hasChanged: () => true })
    ], ControlsModelViewerElement.prototype, "cameraOrbit", void 0);
    __decorate([
        style({
            intrinsics: cameraTargetIntrinsics,
            observeEffects: true,
            updateHandler: $syncCameraTarget
        }),
        property({ type: String, attribute: 'camera-target', hasChanged: () => true })
    ], ControlsModelViewerElement.prototype, "cameraTarget", void 0);
    __decorate([
        style({
            intrinsics: fieldOfViewIntrinsics,
            observeEffects: true,
            updateHandler: $syncFieldOfView
        }),
        property({ type: String, attribute: 'field-of-view', hasChanged: () => true })
    ], ControlsModelViewerElement.prototype, "fieldOfView", void 0);
    __decorate([
        style({
            intrinsics: minCameraOrbitIntrinsics,
            updateHandler: $syncMinCameraOrbit
        }),
        property({ type: String, attribute: 'min-camera-orbit', hasChanged: () => true })
    ], ControlsModelViewerElement.prototype, "minCameraOrbit", void 0);
    __decorate([
        style({
            intrinsics: maxCameraOrbitIntrinsics,
            updateHandler: $syncMaxCameraOrbit
        }),
        property({ type: String, attribute: 'max-camera-orbit', hasChanged: () => true })
    ], ControlsModelViewerElement.prototype, "maxCameraOrbit", void 0);
    __decorate([
        style({
            intrinsics: minFieldOfViewIntrinsics,
            updateHandler: $syncMinFieldOfView
        }),
        property({ type: String, attribute: 'min-field-of-view', hasChanged: () => true })
    ], ControlsModelViewerElement.prototype, "minFieldOfView", void 0);
    __decorate([
        style({ intrinsics: fieldOfViewIntrinsics, updateHandler: $syncMaxFieldOfView }),
        property({ type: String, attribute: 'max-field-of-view', hasChanged: () => true })
    ], ControlsModelViewerElement.prototype, "maxFieldOfView", void 0);
    __decorate([
        property({ type: Number, attribute: 'interaction-prompt-threshold' })
    ], ControlsModelViewerElement.prototype, "interactionPromptThreshold", void 0);
    __decorate([
        property({ type: String, attribute: 'interaction-prompt' })
    ], ControlsModelViewerElement.prototype, "interactionPrompt", void 0);
    __decorate([
        property({ type: String, attribute: 'interaction-prompt-style' })
    ], ControlsModelViewerElement.prototype, "interactionPromptStyle", void 0);
    __decorate([
        property({ type: Number, attribute: 'orbit-sensitivity' })
    ], ControlsModelViewerElement.prototype, "orbitSensitivity", void 0);
    __decorate([
        property({ type: String, attribute: 'touch-action' })
    ], ControlsModelViewerElement.prototype, "touchAction", void 0);
    __decorate([
        property({ type: Boolean, attribute: 'disable-zoom' })
    ], ControlsModelViewerElement.prototype, "disableZoom", void 0);
    __decorate([
        property({ type: Boolean, attribute: 'disable-pan' })
    ], ControlsModelViewerElement.prototype, "disablePan", void 0);
    __decorate([
        property({ type: Boolean, attribute: 'disable-tap' })
    ], ControlsModelViewerElement.prototype, "disableTap", void 0);
    __decorate([
        property({ type: Number, attribute: 'interpolation-decay' })
    ], ControlsModelViewerElement.prototype, "interpolationDecay", void 0);
    return ControlsModelViewerElement;
};
//# sourceMappingURL=controls.js.map