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
var _a, _b, _c, _d;
import { ASTWalker } from './parsers';
const $instances = Symbol('instances');
const $activateListener = Symbol('activateListener');
const $deactivateListener = Symbol('deactivateListener');
const $notifyInstances = Symbol('notifyInstances');
const $notify = Symbol('notify');
const $scrollCallback = Symbol('callback');
/**
 * This internal helper is intended to work as a reference-counting manager of
 * scroll event listeners. Only one scroll listener is ever registered for all
 * instances of the class, and when the last ScrollObserver "disconnects", that
 * event listener is removed. This spares us from thrashing
 * the {add,remove}EventListener API (the binding cost of these methods has been
 * known to show up in performance analyses) as well as potential memory leaks.
 */
class ScrollObserver {
    constructor(callback) {
        this[$scrollCallback] = callback;
    }
    static [$notifyInstances]() {
        for (const instance of ScrollObserver[$instances]) {
            instance[$notify]();
        }
    }
    static [(_a = $instances, $activateListener)]() {
        window.addEventListener('scroll', this[$notifyInstances], { passive: true });
    }
    static [$deactivateListener]() {
        window.removeEventListener('scroll', this[$notifyInstances]);
    }
    /**
     * Listen for scroll events. The configured callback (passed to the
     * constructor) will be invoked for subsequent global scroll events.
     */
    observe() {
        if (ScrollObserver[$instances].size === 0) {
            ScrollObserver[$activateListener]();
        }
        ScrollObserver[$instances].add(this);
    }
    /**
     * Stop listening for scroll events.
     */
    disconnect() {
        ScrollObserver[$instances].delete(this);
        if (ScrollObserver[$instances].size === 0) {
            ScrollObserver[$deactivateListener]();
        }
    }
    [$notify]() {
        this[$scrollCallback]();
    }
    ;
}
ScrollObserver[_a] = new Set();
const $computeStyleCallback = Symbol('computeStyleCallback');
const $astWalker = Symbol('astWalker');
const $dependencies = Symbol('dependencies');
const $onScroll = Symbol('onScroll');
/**
 * The StyleEffector is configured with a callback that will be invoked at the
 * optimal time that some array of CSS expression ASTs ought to be evaluated.
 *
 * For example, our CSS-like expression syntax supports usage of the env()
 * function to incorporate the current top-level scroll position into a CSS
 * expression: env(window-scroll-y).
 *
 * This "environment variable" will change dynamically as the user scrolls the
 * page. If an AST contains such a usage of env(), we would have to evaluate the
 * AST on every frame in order to be sure that the computed style stays up to
 * date.
 *
 * The StyleEffector spares us from evaluating the expressions on every frame by
 * correlating specific parts of an AST with observers of the external effects
 * that they refer to (if any). So, if the AST contains env(window-scroll-y),
 * the StyleEffector manages the lifetime of a global scroll event listener and
 * notifies the user at the optimal time to evaluate the computed style.
 */
export class StyleEffector {
    constructor(callback) {
        this[_b] = {};
        this[_c] = new ASTWalker(['function']);
        this[_d] = () => {
            this[$computeStyleCallback]({ relatedState: 'window-scroll' });
        };
        this[$computeStyleCallback] = callback;
    }
    /**
     * Sets the expressions that govern when the StyleEffector callback will be
     * invoked.
     */
    observeEffectsFor(ast) {
        const newDependencies = {};
        const oldDependencies = this[$dependencies];
        this[$astWalker].walk(ast, functionNode => {
            const { name } = functionNode;
            const firstArgument = functionNode.arguments[0];
            const firstTerm = firstArgument.terms[0];
            if (name.value !== 'env' || firstTerm == null ||
                firstTerm.type !== 'ident') {
                return;
            }
            switch (firstTerm.value) {
                case 'window-scroll-y':
                    if (newDependencies['window-scroll'] == null) {
                        const observer = 'window-scroll' in oldDependencies ?
                            oldDependencies['window-scroll'] :
                            new ScrollObserver(this[$onScroll]);
                        observer.observe();
                        delete oldDependencies['window-scroll'];
                        newDependencies['window-scroll'] = observer;
                    }
                    break;
            }
        });
        for (const environmentState in oldDependencies) {
            const observer = oldDependencies[environmentState];
            observer.disconnect();
        }
        this[$dependencies] = newDependencies;
    }
    /**
     * Disposes of the StyleEffector by disconnecting all observers of external
     * effects.
     */
    dispose() {
        for (const environmentState in this[$dependencies]) {
            const observer = this[$dependencies][environmentState];
            observer.disconnect();
        }
    }
}
_b = $dependencies, _c = $astWalker, _d = $onScroll;
//# sourceMappingURL=style-effector.js.map