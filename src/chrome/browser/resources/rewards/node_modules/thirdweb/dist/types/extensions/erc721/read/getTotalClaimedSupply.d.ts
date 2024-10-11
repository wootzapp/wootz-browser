import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Retrieves the total claimed supply of ERC721 tokens.
 * @param options - The base transaction options.
 * @returns A promise that resolves to the total claimed supply as a bigint.
 * @throws An error if the total claimed supply cannot be retrieved.
 * @extension ERC721
 * @example
 *
 * ```ts
 * import { getTotalClaimedSupply } from "thirdweb/extensions/erc721";
 *
 * const totalClaimedSupply = await getTotalClaimedSupply({
 *  contract,
 * });
 * ```
 */
export declare function getTotalClaimedSupply(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getTotalClaimedSupply.d.ts.map