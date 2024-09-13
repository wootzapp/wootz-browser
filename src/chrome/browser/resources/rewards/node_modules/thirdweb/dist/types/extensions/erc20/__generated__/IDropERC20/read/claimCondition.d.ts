import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xd637ed59";
/**
 * Checks if the `claimCondition` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `claimCondition` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isClaimConditionSupported } from "thirdweb/extensions/erc20";
 * const supported = isClaimConditionSupported(["0x..."]);
 * ```
 */
export declare function isClaimConditionSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the claimCondition function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeClaimConditionResult } from "thirdweb/extensions/erc20";
 * const result = decodeClaimConditionResultResult("...");
 * ```
 */
export declare function decodeClaimConditionResult(result: Hex): readonly [bigint, bigint];
/**
 * Calls the "claimCondition" function on the contract.
 * @param options - The options for the claimCondition function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { claimCondition } from "thirdweb/extensions/erc20";
 *
 * const result = await claimCondition({
 *  contract,
 * });
 *
 * ```
 */
export declare function claimCondition(options: BaseTransactionOptions): Promise<readonly [bigint, bigint]>;
//# sourceMappingURL=claimCondition.d.ts.map