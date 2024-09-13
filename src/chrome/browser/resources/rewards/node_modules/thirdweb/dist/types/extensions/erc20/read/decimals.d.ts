import type { BaseTransactionOptions } from "../../../transaction/types.js";
export { isDecimalsSupported } from "../__generated__/IERC20/read/decimals.js";
/**
 * Retrieves the number of decimal places for a given ERC20 contract.
 * @param options - The options for the transaction.
 * @returns A promise that resolves to the number of decimal places.
 * @extension ERC20
 * @example
 * ```ts
 * import { decimals } from "thirdweb/extensions/erc20";
 *
 * const tokenDecimals = await decimals({ contract });
 * ```
 */
export declare function decimals(options: BaseTransactionOptions): Promise<number>;
//# sourceMappingURL=decimals.d.ts.map