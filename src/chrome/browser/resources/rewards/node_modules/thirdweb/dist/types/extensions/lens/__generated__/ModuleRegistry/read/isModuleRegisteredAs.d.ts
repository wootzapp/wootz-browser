import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isModuleRegisteredAs" function.
 */
export type IsModuleRegisteredAsParams = {
    moduleAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "moduleAddress";
    }>;
    moduleType: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "moduleType";
    }>;
};
export declare const FN_SELECTOR: "0xc2b62fdd";
/**
 * Checks if the `isModuleRegisteredAs` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isModuleRegisteredAs` method is supported.
 * @extension LENS
 * @example
 * ```ts
 * import { isIsModuleRegisteredAsSupported } from "thirdweb/extensions/lens";
 * const supported = isIsModuleRegisteredAsSupported(["0x..."]);
 * ```
 */
export declare function isIsModuleRegisteredAsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isModuleRegisteredAs" function.
 * @param options - The options for the isModuleRegisteredAs function.
 * @returns The encoded ABI parameters.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsModuleRegisteredAsParams } from "thirdweb/extensions/lens";
 * const result = encodeIsModuleRegisteredAsParams({
 *  moduleAddress: ...,
 *  moduleType: ...,
 * });
 * ```
 */
export declare function encodeIsModuleRegisteredAsParams(options: IsModuleRegisteredAsParams): `0x${string}`;
/**
 * Encodes the "isModuleRegisteredAs" function into a Hex string with its parameters.
 * @param options - The options for the isModuleRegisteredAs function.
 * @returns The encoded hexadecimal string.
 * @extension LENS
 * @example
 * ```ts
 * import { encodeIsModuleRegisteredAs } from "thirdweb/extensions/lens";
 * const result = encodeIsModuleRegisteredAs({
 *  moduleAddress: ...,
 *  moduleType: ...,
 * });
 * ```
 */
export declare function encodeIsModuleRegisteredAs(options: IsModuleRegisteredAsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isModuleRegisteredAs function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension LENS
 * @example
 * ```ts
 * import { decodeIsModuleRegisteredAsResult } from "thirdweb/extensions/lens";
 * const result = decodeIsModuleRegisteredAsResultResult("...");
 * ```
 */
export declare function decodeIsModuleRegisteredAsResult(result: Hex): boolean;
/**
 * Calls the "isModuleRegisteredAs" function on the contract.
 * @param options - The options for the isModuleRegisteredAs function.
 * @returns The parsed result of the function call.
 * @extension LENS
 * @example
 * ```ts
 * import { isModuleRegisteredAs } from "thirdweb/extensions/lens";
 *
 * const result = await isModuleRegisteredAs({
 *  contract,
 *  moduleAddress: ...,
 *  moduleType: ...,
 * });
 *
 * ```
 */
export declare function isModuleRegisteredAs(options: BaseTransactionOptions<IsModuleRegisteredAsParams>): Promise<boolean>;
//# sourceMappingURL=isModuleRegisteredAs.d.ts.map