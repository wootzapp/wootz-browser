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
import { StyleEvaluator } from './styles/evaluators';
import { parseExpressions } from './styles/parsers';
import { StyleEffector } from './styles/style-effector';
/**
 * The @style decorator is responsible for coordinating the conversion of a
 * CSS-like string property value into numbers that can be applied to
 * lower-level constructs. It also can optionally manage the lifecycle of a
 * StyleEffector which allows automatic updates for styles that use env() or
 * var() functions.
 *
 * The decorator is configured with Intrinsics and the property key for a
 * method that handles updates. The named update handler is invoked with the
 * result of parsing and evaluating the raw property string value. The format of
 * the evaluated result is derived from the basis of the configured Intrinsics,
 * and is always an array of numbers of fixed length.
 *
 * NOTE: This decorator depends on the property updating mechanism defined by
 * UpdatingElement as exported by the lit-element module. That means it *must*
 * be used in conjunction with the @property decorator, or equivalent
 * JavaScript.
 *
 * Supported configurations are:
 *
 *  - `intrinsics`: An Intrinsics struct that describes how to interpret a
 * serialized style attribute. For more detail on intrinsics see
 * ./styles/evaluators.ts
 *  - `updateHandler`: A string or Symbol that is the key of a method to be
 * invoked with the result of parsing and evaluating a serialized style string.
 *  - `observeEffects`: Optional, if set to true then styles that use env() will
 * cause their update handlers to be invoked every time the corresponding
 * environment variable changes (even if the style attribute itself remains
 * static).
 */
export const style = (config) => {
    const observeEffects = config.observeEffects || false;
    const getIntrinsics = config.intrinsics instanceof Function ?
        config.intrinsics :
        (() => config.intrinsics);
    return (proto, propertyName) => {
        const originalUpdated = proto.updated;
        const originalConnectedCallback = proto.connectedCallback;
        const originalDisconnectedCallback = proto.disconnectedCallback;
        const $styleEffector = Symbol(`${propertyName}StyleEffector`);
        const $styleEvaluator = Symbol(`${propertyName}StyleEvaluator`);
        const $updateEvaluator = Symbol(`${propertyName}UpdateEvaluator`);
        const $evaluateAndSync = Symbol(`${propertyName}EvaluateAndSync`);
        Object.defineProperties(proto, {
            [$styleEffector]: { value: null, writable: true },
            [$styleEvaluator]: { value: null, writable: true },
            [$updateEvaluator]: {
                value: function () {
                    const ast = parseExpressions(this[propertyName]);
                    this[$styleEvaluator] =
                        new StyleEvaluator(ast, getIntrinsics(this));
                    if (this[$styleEffector] == null && observeEffects) {
                        this[$styleEffector] =
                            new StyleEffector(() => this[$evaluateAndSync]());
                    }
                    if (this[$styleEffector] != null) {
                        this[$styleEffector].observeEffectsFor(ast);
                    }
                }
            },
            [$evaluateAndSync]: {
                value: function () {
                    if (this[$styleEvaluator] == null) {
                        return;
                    }
                    const result = this[$styleEvaluator].evaluate();
                    // @see https://github.com/microsoft/TypeScript/pull/30769
                    // @see https://github.com/Microsoft/TypeScript/issues/1863
                    this[config.updateHandler](result);
                }
            },
            updated: {
                value: function (changedProperties) {
                    // Always invoke updates to styles first. This gives a class that
                    // uses this decorator the opportunity to override the effect, or
                    // respond to it, in its own implementation of `updated`.
                    if (changedProperties.has(propertyName)) {
                        this[$updateEvaluator]();
                        this[$evaluateAndSync]();
                    }
                    originalUpdated.call(this, changedProperties);
                }
            },
            connectedCallback: {
                value: function () {
                    originalConnectedCallback.call(this);
                    this.requestUpdate(propertyName, this[propertyName]);
                }
            },
            disconnectedCallback: {
                value: function () {
                    originalDisconnectedCallback.call(this);
                    if (this[$styleEffector] != null) {
                        this[$styleEffector].dispose();
                        this[$styleEffector] = null;
                    }
                }
            }
        });
    };
};
//# sourceMappingURL=decorators.js.map