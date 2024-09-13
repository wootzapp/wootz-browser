var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
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
import { ReactiveElement } from 'lit';
import { property } from 'lit/decorators.js';
import { style } from '../decorators.js';
import { numberNode } from '../styles/parsers.js';
import { timePasses } from '../utilities.js';
const expect = chai.expect;
const $updateFoo = Symbol('updateFoo');
const fooIntrinsics = {
    basis: [numberNode(1, 'm'), numberNode(Math.PI, 'rad')],
    keywords: { auto: [null, numberNode(200, '%')] }
};
class StylableElement extends ReactiveElement {
    constructor() {
        super(...arguments);
        this.foo = '200cm 1rad';
        this.fooUpdates = [];
    }
    [$updateFoo](style) {
        this.fooUpdates.push(style);
    }
}
__decorate([
    style({ intrinsics: fooIntrinsics, updateHandler: $updateFoo }),
    property({ type: String })
], StylableElement.prototype, "foo", void 0);
suite('decorators', () => {
    suite('@style', () => {
        let instance = 0;
        let tagName;
        let element;
        setup(async () => {
            tagName = `stylable-element-${instance++}`;
            customElements.define(tagName, class extends StylableElement {
            });
            element = document.createElement(tagName);
            document.body.insertBefore(element, document.body.firstChild);
            await timePasses();
        });
        teardown(() => {
            document.body.removeChild(element);
        });
        test('invokes the update handler with the parsed default value', () => {
            expect(element.fooUpdates).to.be.eql([[2, 1]]);
        });
        test('invokes the update handler once with a parsed updated value', async () => {
            element.foo = '1m auto';
            await timePasses();
            expect(element.fooUpdates).to.be.eql([[2, 1], [1, 2 * Math.PI]]);
        });
    });
});
//# sourceMappingURL=decorators-spec.js.map