import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xcaa0f92a";
/**
 * Checks if the `getNextTokenId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getNextTokenId` method is supported.
 * @modules SequentialTokenIdERC1155
 * @example
 * ```ts
 * import { SequentialTokenIdERC1155 } from "thirdweb/modules";
 * const supported = SequentialTokenIdERC1155.isGetNextTokenIdSupported(["0x..."]);
 * ```
 */
export declare function isGetNextTokenIdSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getNextTokenId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules SequentialTokenIdERC1155
 * @example
 * ```ts
 * import { SequentialTokenIdERC1155 } from "thirdweb/modules";
 * const result = SequentialTokenIdERC1155.decodeGetNextTokenIdResultResult("...");
 * ```
 */
export declare function decodeGetNextTokenIdResult(result: Hex): bigint;
/**
 * Calls the "getNextTokenId" function on the contract.
 * @param options - The options for the getNextTokenId function.
 * @returns The parsed result of the function call.
 * @modules SequentialTokenIdERC1155
 * @example
 * ```ts
 * import { SequentialTokenIdERC1155 } from "thirdweb/modules";
 *
 * const result = await SequentialTokenIdERC1155.getNextTokenId({
 *  contract,
 * });
 *
 * ```
 */
export declare function getNextTokenId(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getNextTokenId.d.ts.map