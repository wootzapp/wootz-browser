import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xcca5dcb6";
/**
 * Checks if the `isTransferEnabled` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isTransferEnabled` method is supported.
 * @modules TransferableERC1155
 * @example
 * ```ts
 * import { TransferableERC1155 } from "thirdweb/modules";
 * const supported = TransferableERC1155.isIsTransferEnabledSupported(["0x..."]);
 * ```
 */
export declare function isIsTransferEnabledSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the isTransferEnabled function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules TransferableERC1155
 * @example
 * ```ts
 * import { TransferableERC1155 } from "thirdweb/modules";
 * const result = TransferableERC1155.decodeIsTransferEnabledResultResult("...");
 * ```
 */
export declare function decodeIsTransferEnabledResult(result: Hex): boolean;
/**
 * Calls the "isTransferEnabled" function on the contract.
 * @param options - The options for the isTransferEnabled function.
 * @returns The parsed result of the function call.
 * @modules TransferableERC1155
 * @example
 * ```ts
 * import { TransferableERC1155 } from "thirdweb/modules";
 *
 * const result = await TransferableERC1155.isTransferEnabled({
 *  contract,
 * });
 *
 * ```
 */
export declare function isTransferEnabled(options: BaseTransactionOptions): Promise<boolean>;
//# sourceMappingURL=isTransferEnabled.d.ts.map