import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isNewWinningBid" function.
 */
export type IsNewWinningBidParams = {
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_auctionId";
    }>;
    bidAmount: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_bidAmount";
    }>;
};
export declare const FN_SELECTOR: "0x2eb566bd";
/**
 * Checks if the `isNewWinningBid` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isNewWinningBid` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isIsNewWinningBidSupported } from "thirdweb/extensions/marketplace";
 * const supported = isIsNewWinningBidSupported(["0x..."]);
 * ```
 */
export declare function isIsNewWinningBidSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isNewWinningBid" function.
 * @param options - The options for the isNewWinningBid function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsNewWinningBidParams } from "thirdweb/extensions/marketplace";
 * const result = encodeIsNewWinningBidParams({
 *  auctionId: ...,
 *  bidAmount: ...,
 * });
 * ```
 */
export declare function encodeIsNewWinningBidParams(options: IsNewWinningBidParams): `0x${string}`;
/**
 * Encodes the "isNewWinningBid" function into a Hex string with its parameters.
 * @param options - The options for the isNewWinningBid function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsNewWinningBid } from "thirdweb/extensions/marketplace";
 * const result = encodeIsNewWinningBid({
 *  auctionId: ...,
 *  bidAmount: ...,
 * });
 * ```
 */
export declare function encodeIsNewWinningBid(options: IsNewWinningBidParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isNewWinningBid function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeIsNewWinningBidResult } from "thirdweb/extensions/marketplace";
 * const result = decodeIsNewWinningBidResultResult("...");
 * ```
 */
export declare function decodeIsNewWinningBidResult(result: Hex): boolean;
/**
 * Calls the "isNewWinningBid" function on the contract.
 * @param options - The options for the isNewWinningBid function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isNewWinningBid } from "thirdweb/extensions/marketplace";
 *
 * const result = await isNewWinningBid({
 *  contract,
 *  auctionId: ...,
 *  bidAmount: ...,
 * });
 *
 * ```
 */
export declare function isNewWinningBid(options: BaseTransactionOptions<IsNewWinningBidParams>): Promise<boolean>;
//# sourceMappingURL=isNewWinningBid.d.ts.map