import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "encryptedData" function.
 */
export type EncryptedDataParams = {
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "index";
    }>;
};
export declare const FN_SELECTOR: "0xa05112fc";
/**
 * Checks if the `encryptedData` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `encryptedData` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isEncryptedDataSupported } from "thirdweb/extensions/erc721";
 * const supported = isEncryptedDataSupported(["0x..."]);
 * ```
 */
export declare function isEncryptedDataSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "encryptedData" function.
 * @param options - The options for the encryptedData function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeEncryptedDataParams } from "thirdweb/extensions/erc721";
 * const result = encodeEncryptedDataParams({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeEncryptedDataParams(options: EncryptedDataParams): `0x${string}`;
/**
 * Encodes the "encryptedData" function into a Hex string with its parameters.
 * @param options - The options for the encryptedData function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeEncryptedData } from "thirdweb/extensions/erc721";
 * const result = encodeEncryptedData({
 *  index: ...,
 * });
 * ```
 */
export declare function encodeEncryptedData(options: EncryptedDataParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the encryptedData function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeEncryptedDataResult } from "thirdweb/extensions/erc721";
 * const result = decodeEncryptedDataResultResult("...");
 * ```
 */
export declare function decodeEncryptedDataResult(result: Hex): `0x${string}`;
/**
 * Calls the "encryptedData" function on the contract.
 * @param options - The options for the encryptedData function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { encryptedData } from "thirdweb/extensions/erc721";
 *
 * const result = await encryptedData({
 *  contract,
 *  index: ...,
 * });
 *
 * ```
 */
export declare function encryptedData(options: BaseTransactionOptions<EncryptedDataParams>): Promise<`0x${string}`>;
//# sourceMappingURL=encryptedData.d.ts.map