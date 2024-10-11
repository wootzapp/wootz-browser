import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
/**
 * @extension ENS
 */
export type ResolveAvatarOptions = {
    client: ThirdwebClient;
    name: string;
    resolverAddress?: string;
    resolverChain?: Chain;
};
/**
 * Resolves an ENS name to the avatar URL.
 * @param options - The options for resolving an ENS address.
 * @example
 * ```ts
 * import { resolveAvatar } from "thirdweb/extensions/ens";
 * const address = await resolveAvatar({
 *    client,
 *    name: "vitalik.eth",
 * });
 * ```
 * @extension ENS
 * @returns A promise that resolves to the avatar url, or null if not set.
 */
export declare function resolveAvatar(options: ResolveAvatarOptions): Promise<string | null>;
//# sourceMappingURL=resolve-avatar.d.ts.map