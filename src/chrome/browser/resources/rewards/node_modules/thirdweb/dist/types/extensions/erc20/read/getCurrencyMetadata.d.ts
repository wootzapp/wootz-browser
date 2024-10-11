import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * @extension ERC20
 */
export type GetCurrencyMetadataResult = {
    name: string;
    symbol: string;
    decimals: number;
};
/**
 * Retrieves the metadata of a currency.
 * @param options - The options for the transaction.
 * @returns A promise that resolves to an object containing the currency metadata.
 * @extension ERC20
 * @example
 * ```ts
 * import { getCurrencyMetadata } from "thirdweb/extensions/erc20";
 *
 * const currencyMetadata = await getCurrencyMetadata({ contract });
 * ```
 */
export declare function getCurrencyMetadata(options: BaseTransactionOptions): Promise<GetCurrencyMetadataResult>;
//# sourceMappingURL=getCurrencyMetadata.d.ts.map