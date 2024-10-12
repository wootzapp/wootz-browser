import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "allowance" function.
 */
export type AllowanceParams = {
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
    spender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "spender";
    }>;
};
export declare const FN_SELECTOR: "0xdd62ed3e";
/**
 * Checks if the `allowance` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `allowance` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isAllowanceSupported } from "thirdweb/extensions/erc20";
 * const supported = isAllowanceSupported(["0x..."]);
 * ```
 */
export declare function isAllowanceSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "allowance" function.
 * @param options - The options for the allowance function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeAllowanceParams } from "thirdweb/extensions/erc20";
 * const result = encodeAllowanceParams({
 *  owner: ...,
 *  spender: ...,
 * });
 * ```
 */
export declare function encodeAllowanceParams(options: AllowanceParams): `0x${string}`;
/**
 * Encodes the "allowance" function into a Hex string with its parameters.
 * @param options - The options for the allowance function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodeAllowance } from "thirdweb/extensions/erc20";
 * const result = encodeAllowance({
 *  owner: ...,
 *  spender: ...,
 * });
 * ```
 */
export declare function encodeAllowance(options: AllowanceParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the allowance function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeAllowanceResult } from "thirdweb/extensions/erc20";
 * const result = decodeAllowanceResultResult("...");
 * ```
 */
export declare function decodeAllowanceResult(result: Hex): bigint;
/**
 * Calls the "allowance" function on the contract.
 * @param options - The options for the allowance function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { allowance } from "thirdweb/extensions/erc20";
 *
 * const result = await allowance({
 *  contract,
 *  owner: ...,
 *  spender: ...,
 * });
 *
 * ```
 */
export declare function allowance(options: BaseTransactionOptions<AllowanceParams>): Promise<bigint>;
//# sourceMappingURL=allowance.d.ts.map