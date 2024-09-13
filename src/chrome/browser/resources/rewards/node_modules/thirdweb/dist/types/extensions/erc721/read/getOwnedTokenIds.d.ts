import type { BaseTransactionOptions } from "../../../transaction/types.js";
import { type BalanceOfParams } from "../__generated__/IERC721A/read/balanceOf.js";
/**
 * @extension ERC721
 */
export type GetOwnedTokenIdsParams = BalanceOfParams;
/**
 * Retrieves the token IDs owned by a specific address.
 * @param options - The options for retrieving the owned token IDs.
 * @returns A promise that resolves to an array of bigint representing the owned token IDs.
 * @extension ERC721
 * @example
 * ```ts
 * import { getOwnedTokenIds } from "thirdweb/extensions/erc721";
 *
 * const ownedTokenIds = await getOwnedTokenIds({
 *  contract,
 *  owner: "0x1234...",
 * });
 * ```
 */
export declare function getOwnedTokenIds(options: BaseTransactionOptions<GetOwnedTokenIdsParams>): Promise<bigint[]>;
//# sourceMappingURL=getOwnedTokenIds.d.ts.map