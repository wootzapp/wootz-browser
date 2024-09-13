import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isAdmin" function.
 */
export type IsAdminParams = {
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
    }>;
};
export declare const FN_SELECTOR: "0x24d7806c";
/**
 * Checks if the `isAdmin` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isAdmin` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isIsAdminSupported } from "thirdweb/extensions/erc4337";
 * const supported = isIsAdminSupported(["0x..."]);
 * ```
 */
export declare function isIsAdminSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isAdmin" function.
 * @param options - The options for the isAdmin function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIsAdminParams } from "thirdweb/extensions/erc4337";
 * const result = encodeIsAdminParams({
 *  signer: ...,
 * });
 * ```
 */
export declare function encodeIsAdminParams(options: IsAdminParams): `0x${string}`;
/**
 * Encodes the "isAdmin" function into a Hex string with its parameters.
 * @param options - The options for the isAdmin function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIsAdmin } from "thirdweb/extensions/erc4337";
 * const result = encodeIsAdmin({
 *  signer: ...,
 * });
 * ```
 */
export declare function encodeIsAdmin(options: IsAdminParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isAdmin function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4337
 * @example
 * ```ts
 * import { decodeIsAdminResult } from "thirdweb/extensions/erc4337";
 * const result = decodeIsAdminResultResult("...");
 * ```
 */
export declare function decodeIsAdminResult(result: Hex): boolean;
/**
 * Calls the "isAdmin" function on the contract.
 * @param options - The options for the isAdmin function.
 * @returns The parsed result of the function call.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isAdmin } from "thirdweb/extensions/erc4337";
 *
 * const result = await isAdmin({
 *  contract,
 *  signer: ...,
 * });
 *
 * ```
 */
export declare function isAdmin(options: BaseTransactionOptions<IsAdminParams>): Promise<boolean>;
//# sourceMappingURL=isAdmin.d.ts.map