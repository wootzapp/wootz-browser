/* @license
 * Copyright 2020 Google LLC. All Rights Reserved.
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
import { Vector3 } from 'three';
import { CSS2DObject } from 'three/examples/jsm/renderers/CSS2DRenderer.js';
import { normalizeUnit } from '../styles/conversions.js';
import { parseExpressions } from '../styles/parsers.js';
/**
 * The Hotspot object is a reference-counted slot. If decrement() returns true,
 * it should be removed from the tree so it can be garbage-collected.
 */
export class Hotspot extends CSS2DObject {
    constructor(config) {
        super(document.createElement('div'));
        this.normal = new Vector3(0, 1, 0);
        this.initialized = false;
        this.referenceCount = 1;
        this.pivot = document.createElement('div');
        this.slot = document.createElement('slot');
        this.element.classList.add('annotation-wrapper');
        this.slot.name = config.name;
        this.element.appendChild(this.pivot);
        this.pivot.appendChild(this.slot);
        this.updatePosition(config.position);
        this.updateNormal(config.normal);
    }
    get facingCamera() {
        return !this.element.classList.contains('hide');
    }
    /**
     * Sets the hotspot to be in the highly visible foreground state.
     */
    show() {
        if (!this.facingCamera || !this.initialized) {
            this.updateVisibility(true);
        }
    }
    /**
     * Sets the hotspot to be in the diminished background state.
     */
    hide() {
        if (this.facingCamera || !this.initialized) {
            this.updateVisibility(false);
        }
    }
    /**
     * Call this when adding elements to the same slot to keep track.
     */
    increment() {
        this.referenceCount++;
    }
    /**
     * Call this when removing elements from the slot; returns true when the slot
     * is unused.
     */
    decrement() {
        if (this.referenceCount > 0) {
            --this.referenceCount;
        }
        return this.referenceCount === 0;
    }
    /**
     * Change the position of the hotspot to the input string, in the same format
     * as the data-position attribute.
     */
    updatePosition(position) {
        if (position == null)
            return;
        const positionNodes = parseExpressions(position)[0].terms;
        for (let i = 0; i < 3; ++i) {
            this.position.setComponent(i, normalizeUnit(positionNodes[i]).number);
        }
        this.updateMatrixWorld();
    }
    /**
     * Change the hotspot's normal to the input string, in the same format as the
     * data-normal attribute.
     */
    updateNormal(normal) {
        if (normal == null)
            return;
        const normalNodes = parseExpressions(normal)[0].terms;
        for (let i = 0; i < 3; ++i) {
            this.normal.setComponent(i, normalizeUnit(normalNodes[i]).number);
        }
    }
    orient(radians) {
        this.pivot.style.transform = `rotate(${radians}rad)`;
    }
    updateVisibility(show) {
        // NOTE: IE11 doesn't support a second arg for classList.toggle
        if (show) {
            this.element.classList.remove('hide');
        }
        else {
            this.element.classList.add('hide');
        }
        // NOTE: ShadyDOM doesn't support slot.assignedElements, otherwise we could
        // use that here.
        this.slot.assignedNodes().forEach((node) => {
            if (node.nodeType !== Node.ELEMENT_NODE) {
                return;
            }
            const element = node;
            // Visibility attribute can be configured per-node in the hotspot:
            const visibilityAttribute = element.dataset.visibilityAttribute;
            if (visibilityAttribute != null) {
                const attributeName = `data-${visibilityAttribute}`;
                // NOTE: IE11 doesn't support toggleAttribute
                if (show) {
                    element.setAttribute(attributeName, '');
                }
                else {
                    element.removeAttribute(attributeName);
                }
            }
            element.dispatchEvent(new CustomEvent('hotspot-visibility', {
                detail: {
                    visible: show,
                },
            }));
        });
        this.initialized = true;
    }
}
//# sourceMappingURL=Hotspot.js.map