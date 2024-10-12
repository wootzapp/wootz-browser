import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isAuctionExpired" function.
 */
export type IsAuctionExpiredParams = {
    auctionId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "_auctionId";
    }>;
};
export declare const FN_SELECTOR: "0x1389b117";
/**
 * Checks if the `isAuctionExpired` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isAuctionExpired` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isIsAuctionExpiredSupported } from "thirdweb/extensions/marketplace";
 * const supported = isIsAuctionExpiredSupported(["0x..."]);
 * ```
 */
export declare function isIsAuctionExpiredSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isAuctionExpired" function.
 * @param options - The options for the isAuctionExpired function.
 * @returns The encoded ABI parameters.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsAuctionExpiredParams } from "thirdweb/extensions/marketplace";
 * const result = encodeIsAuctionExpiredParams({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeIsAuctionExpiredParams(options: IsAuctionExpiredParams): `0x${string}`;
/**
 * Encodes the "isAuctionExpired" function into a Hex string with its parameters.
 * @param options - The options for the isAuctionExpired function.
 * @returns The encoded hexadecimal string.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { encodeIsAuctionExpired } from "thirdweb/extensions/marketplace";
 * const result = encodeIsAuctionExpired({
 *  auctionId: ...,
 * });
 * ```
 */
export declare function encodeIsAuctionExpired(options: IsAuctionExpiredParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isAuctionExpired function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeIsAuctionExpiredResult } from "thirdweb/extensions/marketplace";
 * const result = decodeIsAuctionExpiredResultResult("...");
 * ```
 */
export declare function decodeIsAuctionExpiredResult(result: Hex): boolean;
/**
 * Calls the "isAuctionExpired" function on the contract.
 * @param options - The options for the isAuctionExpired function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isAuctionExpired } from "thirdweb/extensions/marketplace";
 *
 * const result = await isAuctionExpired({
 *  contract,
 *  auctionId: ...,
 * });
 *
 * ```
 */
export declare function isAuctionExpired(options: BaseTransactionOptions<IsAuctionExpiredParams>): Promise<boolean>;
//# sourceMappingURL=isAuctionExpired.d.ts.map