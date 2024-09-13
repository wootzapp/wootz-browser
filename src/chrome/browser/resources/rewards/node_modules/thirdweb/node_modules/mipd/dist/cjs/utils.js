"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.requestProviders = exports.announceProvider = void 0;
function announceProvider(detail) {
    const event = new CustomEvent('eip6963:announceProvider', { detail: Object.freeze(detail) });
    window.dispatchEvent(event);
    const handler = () => window.dispatchEvent(event);
    window.addEventListener('eip6963:requestProvider', handler);
    return () => window.removeEventListener('eip6963:requestProvider', handler);
}
exports.announceProvider = announceProvider;
function requestProviders(listener) {
    if (typeof window === 'undefined')
        return;
    const handler = (event) => listener(event.detail);
    window.addEventListener('eip6963:announceProvider', handler);
    window.dispatchEvent(new CustomEvent('eip6963:requestProvider'));
    return () => window.removeEventListener('eip6963:announceProvider', handler);
}
exports.requestProviders = requestProviders;
//# sourceMappingURL=utils.js.map