import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isActiveSigner" function.
 */
export type IsActiveSignerParams = {
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
    }>;
};
export declare const FN_SELECTOR: "0x7dff5a79";
/**
 * Checks if the `isActiveSigner` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isActiveSigner` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isIsActiveSignerSupported } from "thirdweb/extensions/erc4337";
 * const supported = isIsActiveSignerSupported(["0x..."]);
 * ```
 */
export declare function isIsActiveSignerSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isActiveSigner" function.
 * @param options - The options for the isActiveSigner function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIsActiveSignerParams } from "thirdweb/extensions/erc4337";
 * const result = encodeIsActiveSignerParams({
 *  signer: ...,
 * });
 * ```
 */
export declare function encodeIsActiveSignerParams(options: IsActiveSignerParams): `0x${string}`;
/**
 * Encodes the "isActiveSigner" function into a Hex string with its parameters.
 * @param options - The options for the isActiveSigner function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeIsActiveSigner } from "thirdweb/extensions/erc4337";
 * const result = encodeIsActiveSigner({
 *  signer: ...,
 * });
 * ```
 */
export declare function encodeIsActiveSigner(options: IsActiveSignerParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isActiveSigner function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4337
 * @example
 * ```ts
 * import { decodeIsActiveSignerResult } from "thirdweb/extensions/erc4337";
 * const result = decodeIsActiveSignerResultResult("...");
 * ```
 */
export declare function decodeIsActiveSignerResult(result: Hex): boolean;
/**
 * Calls the "isActiveSigner" function on the contract.
 * @param options - The options for the isActiveSigner function.
 * @returns The parsed result of the function call.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isActiveSigner } from "thirdweb/extensions/erc4337";
 *
 * const result = await isActiveSigner({
 *  contract,
 *  signer: ...,
 * });
 *
 * ```
 */
export declare function isActiveSigner(options: BaseTransactionOptions<IsActiveSignerParams>): Promise<boolean>;
//# sourceMappingURL=isActiveSigner.d.ts.map