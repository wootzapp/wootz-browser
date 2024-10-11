/**
 * Announces an EIP-1193 Provider.
 */
export function announceProvider(detail) {
    const event = new CustomEvent('eip6963:announceProvider', { detail: Object.freeze(detail) });
    window.dispatchEvent(event);
    const handler = () => window.dispatchEvent(event);
    window.addEventListener('eip6963:requestProvider', handler);
    return () => window.removeEventListener('eip6963:requestProvider', handler);
}
/**
 * Watches for EIP-1193 Providers to be announced.
 */
export function requestProviders(listener) {
    if (typeof window === 'undefined')
        return;
    const handler = (event) => listener(event.detail);
    window.addEventListener('eip6963:announceProvider', handler);
    window.dispatchEvent(new CustomEvent('eip6963:requestProvider'));
    return () => window.removeEventListener('eip6963:announceProvider', handler);
}
//# sourceMappingURL=utils.js.map