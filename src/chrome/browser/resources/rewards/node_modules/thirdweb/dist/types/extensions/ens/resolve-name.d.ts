import type { Address } from "abitype";
import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
/**
 * @extension ENS
 */
export type ResolveNameOptions = {
    client: ThirdwebClient;
    address: Address;
    resolverAddress?: string;
    resolverChain?: Chain;
};
/**
 * Resolves the primary name for a specified address.
 * @param options - The options for resolving an ENS address.
 * @example
 * ```ts
 * import { resolveName } from "thirdweb/extensions/ens";
 * const name = await resolveName({
 *    client,
 *    address: "0x1234...",
 * });
 * ```
 * @extension ENS
 * @returns A promise that resolves to the Ethereum address.
 */
export declare function resolveName(options: ResolveNameOptions): Promise<string | null>;
//# sourceMappingURL=resolve-name.d.ts.map