import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isModuleRegistered" function.
 */
export type IsModuleRegisteredParams = {
    moduleAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "moduleAddress";
    }>;
};
export declare const FN_SELECTOR: "0x1c5ebe2f";
/**
 * Checks if the `isModuleRegistered` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isModuleRegistered` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isIsModuleRegisteredSupported } from "thirdweb/extensions/lens";
 * const supported = isIsModuleRegisteredSupported(["0x..."]);
 * ```
 */
export declare function isIsModuleRegisteredSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isModuleRegistered" function.
 * @param options - The options for the isModuleRegistered function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsModuleRegisteredParams } from "thirdweb/extensions/lens";
 * const result = encodeIsModuleRegisteredParams({
 *  moduleAddress: ...,
 * });
 * ```
 */
export declare function encodeIsModuleRegisteredParams(options: IsModuleRegisteredParams): `0x${string}`;
/**
 * Encodes the "isModuleRegistered" function into a Hex string with its parameters.
 * @param options - The options for the isModuleRegistered function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsModuleRegistered } from "thirdweb/extensions/lens";
 * const result = encodeIsModuleRegistered({
 *  moduleAddress: ...,
 * });
 * ```
 */
export declare function encodeIsModuleRegistered(options: IsModuleRegisteredParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isModuleRegistered function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeIsModuleRegisteredResult } from "thirdweb/extensions/lens";
 * const result = decodeIsModuleRegisteredResultResult("...");
 * ```
 */
export declare function decodeIsModuleRegisteredResult(result: Hex): boolean;
/**
 * Calls the "isModuleRegistered" function on the contract.
 * @param options - The options for the isModuleRegistered function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { isModuleRegistered } from "thirdweb/extensions/lens";
 *
 * const result = await isModuleRegistered({
 *  contract,
 *  moduleAddress: ...,
 * });
 *
 * ```
 */
export declare function isModuleRegistered(options: BaseTransactionOptions<IsModuleRegisteredParams>): Promise<boolean>;
//# sourceMappingURL=isModuleRegistered.d.ts.map