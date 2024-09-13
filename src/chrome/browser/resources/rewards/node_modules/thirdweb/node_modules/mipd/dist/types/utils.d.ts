import type { EIP1193Provider } from './register.js';
import type { EIP6963ProviderDetail } from './types.js';
export type AnnounceProviderParameters = EIP6963ProviderDetail<EIP1193Provider, string>;
export type AnnounceProviderReturnType = () => void;
/**
 * Announces an EIP-1193 Provider.
 */
export declare function announceProvider(detail: AnnounceProviderParameters): AnnounceProviderReturnType;
export type RequestProvidersParameters = (providerDetail: EIP6963ProviderDetail) => void;
export type RequestProvidersReturnType = (() => void) | undefined;
/**
 * Watches for EIP-1193 Providers to be announced.
 */
export declare function requestProviders(listener: RequestProvidersParameters): RequestProvidersReturnType;
//# sourceMappingURL=utils.d.ts.map