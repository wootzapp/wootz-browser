import type { Address } from "abitype";
import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
/**
 * @extension ENS
 */
export type ResolveL2NameOptions = {
    client: ThirdwebClient;
    address: Address;
    resolverAddress: string;
    resolverChain: Chain;
};
/**
 * Resolves the L2 name for a specified address.
 * @param options - The options for resolving an L2 ENS address.
 * @example
 * ```ts
 * import { resolveL2Name } from "thirdweb/extensions/ens";
 * const name = await resolveL2Name({
 *    client,
 *    address: "0x1234...",
 *    resolverAddress: "0x...",
 *    resolverChain: base,
 * });
 * ```
 *
 * Resolve a Basename.
 * ```ts
 * import { resolveL2Name, BASENAME_RESOLVER_ADDRESS } from "thirdweb/extensions/ens";
 * import { base } from "thirdweb/chains";
 * const name = await resolveL2Name({
 *    client,
 *    address: "0x1234...",
 *    resolverAddress: BASENAME_RESOLVER_ADDRESS,
 *    resolverChain: base,
 * });
 * ```
 * @extension ENS
 * @returns A promise that resolves to the Ethereum address.
 */
export declare function resolveL2Name(options: ResolveL2NameOptions): Promise<string | null>;
//# sourceMappingURL=resolve-l2-name.d.ts.map