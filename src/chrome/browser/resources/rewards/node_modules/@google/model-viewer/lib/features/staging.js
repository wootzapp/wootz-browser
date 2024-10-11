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
import { style } from '../decorators.js';
import { $getModelIsVisible, $renderer, $scene, $tick } from '../model-viewer-base.js';
import { degreesToRadians } from '../styles/conversions.js';
import { numberNode } from '../styles/parsers.js';
// How much the model will rotate per
// second in radians:
const DEFAULT_ROTATION_SPEED = Math.PI / 32;
export const AUTO_ROTATE_DELAY_DEFAULT = 3000;
const rotationRateIntrinsics = {
    basis: [degreesToRadians(numberNode(DEFAULT_ROTATION_SPEED, 'rad'))],
    keywords: { auto: [null] }
};
const $autoRotateStartTime = Symbol('autoRotateStartTime');
const $radiansPerSecond = Symbol('radiansPerSecond');
const $syncRotationRate = Symbol('syncRotationRate');
const $onCameraChange = Symbol('onCameraChange');
export const StagingMixin = (ModelViewerElement) => {
    var _a, _b, _c;
    class StagingModelViewerElement extends ModelViewerElement {
        constructor() {
            super(...arguments);
            this.autoRotate = false;
            this.autoRotateDelay = AUTO_ROTATE_DELAY_DEFAULT;
            this.rotationPerSecond = 'auto';
            this[_a] = performance.now();
            this[_b] = 0;
            this[_c] = (event) => {
                if (!this.autoRotate) {
                    return;
                }
                if (event.detail.source === 'user-interaction') {
                    this[$autoRotateStartTime] = performance.now();
                }
            };
        }
        connectedCallback() {
            super.connectedCallback();
            this.addEventListener('camera-change', this[$onCameraChange]);
            this[$autoRotateStartTime] = performance.now();
        }
        disconnectedCallback() {
            super.disconnectedCallback();
            this.removeEventListener('camera-change', this[$onCameraChange]);
            this[$autoRotateStartTime] = performance.now();
        }
        updated(changedProperties) {
            super.updated(changedProperties);
            if (changedProperties.has('autoRotate')) {
                this[$autoRotateStartTime] = performance.now();
            }
        }
        [(_a = $autoRotateStartTime, _b = $radiansPerSecond, $syncRotationRate)](style) {
            this[$radiansPerSecond] = style[0];
        }
        [$tick](time, delta) {
            super[$tick](time, delta);
            if (!this.autoRotate || !this[$getModelIsVisible]() ||
                this[$renderer].isPresenting) {
                return;
            }
            const rotationDelta = Math.min(delta, time - this[$autoRotateStartTime] - this.autoRotateDelay);
            if (rotationDelta > 0) {
                this[$scene].yaw = this.turntableRotation +
                    this[$radiansPerSecond] * rotationDelta * 0.001;
            }
        }
        get turntableRotation() {
            return this[$scene].yaw;
        }
        resetTurntableRotation(theta = 0) {
            this[$scene].yaw = theta;
        }
    }
    _c = $onCameraChange;
    __decorate([
        property({ type: Boolean, attribute: 'auto-rotate' })
    ], StagingModelViewerElement.prototype, "autoRotate", void 0);
    __decorate([
        property({ type: Number, attribute: 'auto-rotate-delay' })
    ], StagingModelViewerElement.prototype, "autoRotateDelay", void 0);
    __decorate([
        style({ intrinsics: rotationRateIntrinsics, updateHandler: $syncRotationRate }),
        property({ type: String, attribute: 'rotation-per-second' })
    ], StagingModelViewerElement.prototype, "rotationPerSecond", void 0);
    return StagingModelViewerElement;
};
//# sourceMappingURL=staging.js.map