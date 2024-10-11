import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isClaimed" function.
 */
export type IsClaimedParams = {
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_receiver";
    }>;
    token: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_token";
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_tokenId";
    }>;
};
export declare const FN_SELECTOR: "0xd12acf73";
/**
 * Checks if the `isClaimed` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isClaimed` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isIsClaimedSupported } from "thirdweb/extensions/airdrop";
 * const supported = isIsClaimedSupported(["0x..."]);
 * ```
 */
export declare function isIsClaimedSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isClaimed" function.
 * @param options - The options for the isClaimed function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeIsClaimedParams } from "thirdweb/extensions/airdrop";
 * const result = encodeIsClaimedParams({
 *  receiver: ...,
 *  token: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeIsClaimedParams(options: IsClaimedParams): `0x${string}`;
/**
 * Encodes the "isClaimed" function into a Hex string with its parameters.
 * @param options - The options for the isClaimed function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeIsClaimed } from "thirdweb/extensions/airdrop";
 * const result = encodeIsClaimed({
 *  receiver: ...,
 *  token: ...,
 *  tokenId: ...,
 * });
 * ```
 */
export declare function encodeIsClaimed(options: IsClaimedParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isClaimed function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { decodeIsClaimedResult } from "thirdweb/extensions/airdrop";
 * const result = decodeIsClaimedResultResult("...");
 * ```
 */
export declare function decodeIsClaimedResult(result: Hex): boolean;
/**
 * Calls the "isClaimed" function on the contract.
 * @param options - The options for the isClaimed function.
 * @returns The parsed result of the function call.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isClaimed } from "thirdweb/extensions/airdrop";
 *
 * const result = await isClaimed({
 *  contract,
 *  receiver: ...,
 *  token: ...,
 *  tokenId: ...,
 * });
 *
 * ```
 */
export declare function isClaimed(options: BaseTransactionOptions<IsClaimedParams>): Promise<boolean>;
//# sourceMappingURL=isClaimed.d.ts.map