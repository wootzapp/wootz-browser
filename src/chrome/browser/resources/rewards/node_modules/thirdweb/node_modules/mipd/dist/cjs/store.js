"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.createStore = void 0;
const utils_js_1 = require("./utils.js");
function createStore() {
    const listeners = new Set();
    let providerDetails = [];
    const request = () => (0, utils_js_1.requestProviders)((providerDetail) => {
        if (providerDetails.some(({ info }) => info.uuid === providerDetail.info.uuid))
            return;
        providerDetails = [...providerDetails, providerDetail];
        listeners.forEach((listener) => listener(providerDetails, { added: [providerDetail] }));
    });
    let unwatch = request();
    return {
        _listeners() {
            return listeners;
        },
        clear() {
            listeners.forEach((listener) => listener([], { removed: [...providerDetails] }));
            providerDetails = [];
        },
        destroy() {
            this.clear();
            listeners.clear();
            unwatch?.();
        },
        findProvider({ rdns }) {
            return providerDetails.find((providerDetail) => providerDetail.info.rdns === rdns);
        },
        getProviders() {
            return providerDetails;
        },
        reset() {
            this.clear();
            unwatch?.();
            unwatch = request();
        },
        subscribe(listener, { emitImmediately } = {}) {
            listeners.add(listener);
            if (emitImmediately)
                listener(providerDetails, { added: providerDetails });
            return () => listeners.delete(listener);
        },
    };
}
exports.createStore = createStore;
//# sourceMappingURL=store.js.map