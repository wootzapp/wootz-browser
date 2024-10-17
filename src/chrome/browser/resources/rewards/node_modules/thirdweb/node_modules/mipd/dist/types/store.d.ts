import type { Rdns } from './register.js';
import type { EIP6963ProviderDetail } from './types.js';
export type Listener = (providerDetails: readonly EIP6963ProviderDetail[], meta?: {
    added?: readonly EIP6963ProviderDetail[] | undefined;
    removed?: readonly EIP6963ProviderDetail[] | undefined;
} | undefined) => void;
export type Store = {
    /**
     * Clears the store, including all provider details.
     */
    clear(): void;
    /**
     * Destroys the store, including all provider details and listeners.
     */
    destroy(): void;
    /**
     * Finds a provider detail by its RDNS (Reverse Domain Name Identifier).
     */
    findProvider(args: {
        rdns: Rdns;
    }): EIP6963ProviderDetail | undefined;
    /**
     * Returns all provider details that have been emitted.
     */
    getProviders(): readonly EIP6963ProviderDetail[];
    /**
     * Resets the store, and emits an event to request provider details.
     */
    reset(): void;
    /**
     * Subscribes to emitted provider details.
     */
    subscribe(listener: Listener, args?: {
        emitImmediately?: boolean | undefined;
    } | undefined): () => void;
    /**
     * @internal
     * Current state of listening listeners.
     */
    _listeners(): Set<Listener>;
};
export declare function createStore(): Store;
//# sourceMappingURL=store.d.ts.map