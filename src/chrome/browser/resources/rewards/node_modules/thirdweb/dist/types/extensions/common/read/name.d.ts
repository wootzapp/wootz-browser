import type { BaseTransactionOptions } from "../../../transaction/types.js";
export { isNameSupported } from "../__generated__/IContractMetadata/read/name.js";
/**
 * Retrieves the name associated with the given contract.
 * @param options - The options for the transaction.
 * @returns A promise that resolves to the name associated with the contract.
 * @extension COMMON
 * @example
 * ```ts
 * import { name } from "thirdweb/extensions/common";
 *
 * const contractName = await name({ contract });
 * ```
 */
export declare function name(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=name.d.ts.map