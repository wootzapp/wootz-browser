/**
 * This mixin function is designed to be applied to a class that inherits
 * from HTMLElement. It makes it easy for a custom element to coordinate with
 * the :focus-visible polyfill.
 *
 * NOTE(cdata): The code here was adapted from an example proposed with the
 * introduction of ShadowDOM support in the :focus-visible polyfill.
 *
 * @see https://github.com/WICG/focus-visible/pull/196
 * @param {Function} SuperClass The base class implementation to decorate with
 * implementation that coordinates with the :focus-visible polyfill
 */
export const FocusVisiblePolyfillMixin = (SuperClass) => {
    var _a;
    const coordinateWithPolyfill = (instance) => {
        // If there is no shadow root, there is no need to coordinate with
        // the polyfill. If we already coordinated with the polyfill, we can
        // skip subsequent invocations:
        if (instance.shadowRoot == null ||
            instance.hasAttribute('data-js-focus-visible')) {
            return () => { };
        }
        // The polyfill might already be loaded. If so, we can apply it to
        // the shadow root immediately:
        if (self.applyFocusVisiblePolyfill) {
            self.applyFocusVisiblePolyfill(instance.shadowRoot);
        }
        else {
            const coordinationHandler = () => {
                self.applyFocusVisiblePolyfill(instance.shadowRoot);
            };
            // Otherwise, wait for the polyfill to be loaded lazily. It might
            // never be loaded, but if it is then we can apply it to the
            // shadow root at the appropriate time by waiting for the ready
            // event:
            self.addEventListener('focus-visible-polyfill-ready', coordinationHandler, { once: true });
            return () => {
                self.removeEventListener('focus-visible-polyfill-ready', coordinationHandler);
            };
        }
        return () => { };
    };
    const $endPolyfillCoordination = Symbol('endPolyfillCoordination');
    // IE11 doesn't natively support custom elements or JavaScript class
    // syntax The mixin implementation assumes that the user will take the
    // appropriate steps to support both:
    class FocusVisibleCoordinator extends SuperClass {
        constructor() {
            super(...arguments);
            this[_a] = null;
        }
        // Attempt to coordinate with the polyfill when connected to the
        // document:
        connectedCallback() {
            super.connectedCallback && super.connectedCallback();
            if (this[$endPolyfillCoordination] == null) {
                this[$endPolyfillCoordination] = coordinateWithPolyfill(this);
            }
        }
        disconnectedCallback() {
            super.disconnectedCallback && super.disconnectedCallback();
            // It's important to remove the polyfill event listener when we
            // disconnect, otherwise we will leak the whole element via window:
            if (this[$endPolyfillCoordination] != null) {
                this[$endPolyfillCoordination]();
                this[$endPolyfillCoordination] = null;
            }
        }
    }
    _a = $endPolyfillCoordination;
    ;
    return FocusVisibleCoordinator;
};
//# sourceMappingURL=focus-visible.js.map