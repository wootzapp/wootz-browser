import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Retrieves the total unclaimed supply of ERC721 tokens.
 * @param options - The base transaction options.
 * @returns A promise that resolves to the total unclaimed supply as a bigint.
 * @extension ERC721
 * @example
 *
 * ```ts
 * import { getTotalUnclaimedSupply } from "thirdweb/extensions/erc721";
 *
 * const totalUnclaimedSupply = await getTotalUnclaimedSupply({
 *  contract,
 * });
 */
export declare function getTotalUnclaimedSupply(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getTotalUnclaimedSupply.d.ts.map