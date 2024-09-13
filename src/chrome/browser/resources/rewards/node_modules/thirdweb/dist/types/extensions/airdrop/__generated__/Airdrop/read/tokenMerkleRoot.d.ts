import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "tokenMerkleRoot" function.
 */
export type TokenMerkleRootParams = {
    tokenAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenAddress";
    }>;
};
export declare const FN_SELECTOR: "0x95f5c120";
/**
 * Checks if the `tokenMerkleRoot` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `tokenMerkleRoot` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isTokenMerkleRootSupported } from "thirdweb/extensions/airdrop";
 * const supported = isTokenMerkleRootSupported(["0x..."]);
 * ```
 */
export declare function isTokenMerkleRootSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "tokenMerkleRoot" function.
 * @param options - The options for the tokenMerkleRoot function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeTokenMerkleRootParams } from "thirdweb/extensions/airdrop";
 * const result = encodeTokenMerkleRootParams({
 *  tokenAddress: ...,
 * });
 * ```
 */
export declare function encodeTokenMerkleRootParams(options: TokenMerkleRootParams): `0x${string}`;
/**
 * Encodes the "tokenMerkleRoot" function into a Hex string with its parameters.
 * @param options - The options for the tokenMerkleRoot function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeTokenMerkleRoot } from "thirdweb/extensions/airdrop";
 * const result = encodeTokenMerkleRoot({
 *  tokenAddress: ...,
 * });
 * ```
 */
export declare function encodeTokenMerkleRoot(options: TokenMerkleRootParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the tokenMerkleRoot function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { decodeTokenMerkleRootResult } from "thirdweb/extensions/airdrop";
 * const result = decodeTokenMerkleRootResultResult("...");
 * ```
 */
export declare function decodeTokenMerkleRootResult(result: Hex): `0x${string}`;
/**
 * Calls the "tokenMerkleRoot" function on the contract.
 * @param options - The options for the tokenMerkleRoot function.
 * @returns The parsed result of the function call.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { tokenMerkleRoot } from "thirdweb/extensions/airdrop";
 *
 * const result = await tokenMerkleRoot({
 *  contract,
 *  tokenAddress: ...,
 * });
 *
 * ```
 */
export declare function tokenMerkleRoot(options: BaseTransactionOptions<TokenMerkleRootParams>): Promise<`0x${string}`>;
//# sourceMappingURL=tokenMerkleRoot.d.ts.map