import type { BaseTransactionOptions } from "../../../transaction/types.js";
export type TokenMintParams = {
    to: string;
} & ({
    quantity: string;
} | {
    quantityWei: bigint;
});
/**
 * Mints ERC20 tokens to a specified address via a MintableERC20 module.
 * @param options The options for minting tokens.
 * @returns A transaction to mint tokens.
 * @example
 * ```typescript
 * import { MintableERC20 } from "thirdweb/modules";
 *
 * const transaction = MintableERC20.mintWithRole({
 *   contract,
 *   to: "0x...", // Address to mint tokens to
 *   quantity: 2, // Amount of tokens to mint (in decimals)
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 * @modules MintableERC20
 */
export declare function mintWithRole(options: BaseTransactionOptions<TokenMintParams>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=mintWithRole.d.ts.map