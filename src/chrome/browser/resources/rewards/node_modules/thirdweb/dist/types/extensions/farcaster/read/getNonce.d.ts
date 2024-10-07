import type { Address } from "abitype";
import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * @extension FARCASTER
 */
export type GetNonceParams = {
    client: ThirdwebClient;
    address: Address;
    chain?: Chain;
    disableCache?: boolean;
};
/**
 * Retrieves the current key gateway nonce for an account.
 * @param options - Parameters to pass to the function.
 * @returns A promise that resolves to the current nonce.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getNonce } from "thirdweb/extensions/farcaster";
 *
 * const nonce = await getNonce({
 *  client,
 * 	address,
 * });
 * ```
 */
export declare function getNonce(options: GetNonceParams): Promise<bigint>;
//# sourceMappingURL=getNonce.d.ts.map