import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xc68907de";
/**
 * Checks if the `getActiveClaimConditionId` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getActiveClaimConditionId` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isGetActiveClaimConditionIdSupported } from "thirdweb/extensions/erc20";
 * const supported = isGetActiveClaimConditionIdSupported(["0x..."]);
 * ```
 */
export declare function isGetActiveClaimConditionIdSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getActiveClaimConditionId function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeGetActiveClaimConditionIdResult } from "thirdweb/extensions/erc20";
 * const result = decodeGetActiveClaimConditionIdResultResult("...");
 * ```
 */
export declare function decodeGetActiveClaimConditionIdResult(result: Hex): bigint;
/**
 * Calls the "getActiveClaimConditionId" function on the contract.
 * @param options - The options for the getActiveClaimConditionId function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { getActiveClaimConditionId } from "thirdweb/extensions/erc20";
 *
 * const result = await getActiveClaimConditionId({
 *  contract,
 * });
 *
 * ```
 */
export declare function getActiveClaimConditionId(options: BaseTransactionOptions): Promise<bigint>;
//# sourceMappingURL=getActiveClaimConditionId.d.ts.map