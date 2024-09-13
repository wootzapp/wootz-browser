import type { BaseTransactionOptions } from "../../../transaction/types.js";
export { isSymbolSupported } from "../__generated__/IContractMetadata/read/symbol.js";
/**
 * Retrieves the name associated with the given contract.
 * @param options - The options for the transaction.
 * @returns A promise that resolves to the name associated with the contract.
 * @extension COMMON
 * @example
 * ```ts
 * import { symbol } from "thirdweb/extensions/common";
 *
 * const contractSymbol = await symbol({ contract });
 * ```
 */
export declare function symbol(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=symbol.d.ts.map