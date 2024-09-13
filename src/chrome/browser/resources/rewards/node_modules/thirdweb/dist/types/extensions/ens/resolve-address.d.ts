import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
/**
 * @extension ENS
 */
export type ResolveAddressOptions = {
    client: ThirdwebClient;
    name: string;
    resolverAddress?: string;
    resolverChain?: Chain;
};
/**
 * Resolves an ENS name to an Ethereum address.
 * @param options - The options for resolving an ENS address.
 * @example
 * ```ts
 * import { resolveAddress } from "thirdweb/extensions/ens";
 * const address = await resolveAddress({
 *    client,
 *    name: "vitalik.eth",
 * });
 * ```
 *
 * Resolve an address to a Basename.
 * ```ts
 * import { resolveAddress, BASENAME_RESOLVER_ADDRESS } from "thirdweb/extensions/ens";
 * import { base } from "thirdweb/chains";
 * const address = await resolveAddress({
 *    client,
 *    name: "myk.base.eth",
 *    resolverAddress: BASENAME_RESOLVER_ADDRESS,
 *    resolverChain: base,
 * });
 * ```
 * @extension ENS
 * @returns A promise that resolves to the Ethereum address.
 */
export declare function resolveAddress(options: ResolveAddressOptions): Promise<`0x${string}`>;
//# sourceMappingURL=resolve-address.d.ts.map