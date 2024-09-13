import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "encryptDecrypt" function.
 */
export type EncryptDecryptParams = {
    data: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "data";
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
    }>;
};
export declare const FN_SELECTOR: "0xe7150322";
/**
 * Checks if the `encryptDecrypt` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `encryptDecrypt` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isEncryptDecryptSupported } from "thirdweb/extensions/erc721";
 * const supported = isEncryptDecryptSupported(["0x..."]);
 * ```
 */
export declare function isEncryptDecryptSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "encryptDecrypt" function.
 * @param options - The options for the encryptDecrypt function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeEncryptDecryptParams } from "thirdweb/extensions/erc721";
 * const result = encodeEncryptDecryptParams({
 *  data: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeEncryptDecryptParams(options: EncryptDecryptParams): `0x${string}`;
/**
 * Encodes the "encryptDecrypt" function into a Hex string with its parameters.
 * @param options - The options for the encryptDecrypt function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeEncryptDecrypt } from "thirdweb/extensions/erc721";
 * const result = encodeEncryptDecrypt({
 *  data: ...,
 *  key: ...,
 * });
 * ```
 */
export declare function encodeEncryptDecrypt(options: EncryptDecryptParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the encryptDecrypt function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC721
 * @example
 * ```ts
 * import { decodeEncryptDecryptResult } from "thirdweb/extensions/erc721";
 * const result = decodeEncryptDecryptResultResult("...");
 * ```
 */
export declare function decodeEncryptDecryptResult(result: Hex): `0x${string}`;
/**
 * Calls the "encryptDecrypt" function on the contract.
 * @param options - The options for the encryptDecrypt function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { encryptDecrypt } from "thirdweb/extensions/erc721";
 *
 * const result = await encryptDecrypt({
 *  contract,
 *  data: ...,
 *  key: ...,
 * });
 *
 * ```
 */
export declare function encryptDecrypt(options: BaseTransactionOptions<EncryptDecryptParams>): Promise<`0x${string}`>;
//# sourceMappingURL=encryptDecrypt.d.ts.map