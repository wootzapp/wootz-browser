import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isRegistered" function.
 */
export type IsRegisteredParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0xc3c5a547";
/**
 * Checks if the `isRegistered` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isRegistered` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isIsRegisteredSupported } from "thirdweb/extensions/erc4337";
 * const supported = isIsRegisteredSupported(["0x..."]);
 * ```
 */
export declare function isIsRegisteredSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isRegistered" function.
 * @param options - The options for the isRegistered function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIsRegisteredParams } from "thirdweb/extensions/erc4337";
 * const result = encodeIsRegisteredParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeIsRegisteredParams(options: IsRegisteredParams): `0x${string}`;
/**
 * Encodes the "isRegistered" function into a Hex string with its parameters.
 * @param options - The options for the isRegistered function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIsRegistered } from "thirdweb/extensions/erc4337";
 * const result = encodeIsRegistered({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeIsRegistered(options: IsRegisteredParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isRegistered function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4337
 * @example
 * ```ts
 * import { decodeIsRegisteredResult } from "thirdweb/extensions/erc4337";
 * const result = decodeIsRegisteredResultResult("...");
 * ```
 */
export declare function decodeIsRegisteredResult(result: Hex): boolean;
/**
 * Calls the "isRegistered" function on the contract.
 * @param options - The options for the isRegistered function.
 * @returns The parsed result of the function call.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isRegistered } from "thirdweb/extensions/erc4337";
 *
 * const result = await isRegistered({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function isRegistered(options: BaseTransactionOptions<IsRegisteredParams>): Promise<boolean>;
//# sourceMappingURL=isRegistered.d.ts.map