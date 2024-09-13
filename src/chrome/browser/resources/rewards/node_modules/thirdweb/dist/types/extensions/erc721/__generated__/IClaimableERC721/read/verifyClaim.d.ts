import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "verifyClaim" function.
 */
export type VerifyClaimParams = {
    claimer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_claimer";
    }>;
    quantity: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_quantity";
    }>;
};
export declare const FN_SELECTOR: "0x2f92023a";
/**
 * Checks if the `verifyClaim` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `verifyClaim` method is supported.
 * @extension ERC721
 * @example
 * ```ts
 * import { isVerifyClaimSupported } from "thirdweb/extensions/erc721";
 * const supported = isVerifyClaimSupported(["0x..."]);
 * ```
 */
export declare function isVerifyClaimSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "verifyClaim" function.
 * @param options - The options for the verifyClaim function.
 * @returns The encoded ABI parameters.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeVerifyClaimParams } from "thirdweb/extensions/erc721";
 * const result = encodeVerifyClaimParams({
 *  claimer: ...,
 *  quantity: ...,
 * });
 * ```
 */
export declare function encodeVerifyClaimParams(options: VerifyClaimParams): `0x${string}`;
/**
 * Encodes the "verifyClaim" function into a Hex string with its parameters.
 * @param options - The options for the verifyClaim function.
 * @returns The encoded hexadecimal string.
 * @extension ERC721
 * @example
 * ```ts
 * import { encodeVerifyClaim } from "thirdweb/extensions/erc721";
 * const result = encodeVerifyClaim({
 *  claimer: ...,
 *  quantity: ...,
 * });
 * ```
 */
export declare function encodeVerifyClaim(options: VerifyClaimParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Calls the "verifyClaim" function on the contract.
 * @param options - The options for the verifyClaim function.
 * @returns The parsed result of the function call.
 * @extension ERC721
 * @example
 * ```ts
 * import { verifyClaim } from "thirdweb/extensions/erc721";
 *
 * const result = await verifyClaim({
 *  contract,
 *  claimer: ...,
 *  quantity: ...,
 * });
 *
 * ```
 */
export declare function verifyClaim(options: BaseTransactionOptions<VerifyClaimParams>): Promise<never>;
//# sourceMappingURL=verifyClaim.d.ts.map