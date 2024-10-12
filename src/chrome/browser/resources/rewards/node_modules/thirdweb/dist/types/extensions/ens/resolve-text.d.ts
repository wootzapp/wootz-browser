import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
/**
 * @extension ENS
 */
export type ResolveTextOptions = {
    client: ThirdwebClient;
    name: string;
    key: string;
    resolverAddress?: string;
    resolverChain?: Chain;
};
/**
 * Resolves an ENS name and key to the specified record.
 * @param options - The options for resolving an ENS address.
 * @example
 * ```ts
 * import { resolveText } from "thirdweb/extensions/ens";
 * const twitterUsername = await resolveText({
 *    client,
 *    name: "vitalik.eth",
 *    key: "com.twitter"
 * });
 * ```
 * @extension ENS
 * @returns A promise that resolves to the text record.
 */
export declare function resolveText(options: ResolveTextOptions): Promise<string | null>;
//# sourceMappingURL=resolve-text.d.ts.map