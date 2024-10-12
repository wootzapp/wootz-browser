import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getPublisherProfileUri" function.
 */
export type GetPublisherProfileUriParams = {
    publisher: AbiParameterToPrimitiveType<{
        type: "address";
        name: "publisher";
    }>;
};
export declare const FN_SELECTOR: "0x4f781675";
/**
 * Checks if the `getPublisherProfileUri` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getPublisherProfileUri` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isGetPublisherProfileUriSupported } from "thirdweb/extensions/thirdweb";
 * const supported = isGetPublisherProfileUriSupported(["0x..."]);
 * ```
 */
export declare function isGetPublisherProfileUriSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getPublisherProfileUri" function.
 * @param options - The options for the getPublisherProfileUri function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetPublisherProfileUriParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetPublisherProfileUriParams({
 *  publisher: ...,
 * });
 * ```
 */
export declare function encodeGetPublisherProfileUriParams(options: GetPublisherProfileUriParams): `0x${string}`;
/**
 * Encodes the "getPublisherProfileUri" function into a Hex string with its parameters.
 * @param options - The options for the getPublisherProfileUri function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeGetPublisherProfileUri } from "thirdweb/extensions/thirdweb";
 * const result = encodeGetPublisherProfileUri({
 *  publisher: ...,
 * });
 * ```
 */
export declare function encodeGetPublisherProfileUri(options: GetPublisherProfileUriParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getPublisherProfileUri function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeGetPublisherProfileUriResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeGetPublisherProfileUriResultResult("...");
 * ```
 */
export declare function decodeGetPublisherProfileUriResult(result: Hex): string;
/**
 * Calls the "getPublisherProfileUri" function on the contract.
 * @param options - The options for the getPublisherProfileUri function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getPublisherProfileUri } from "thirdweb/extensions/thirdweb";
 *
 * const result = await getPublisherProfileUri({
 *  contract,
 *  publisher: ...,
 * });
 *
 * ```
 */
export declare function getPublisherProfileUri(options: BaseTransactionOptions<GetPublisherProfileUriParams>): Promise<string>;
//# sourceMappingURL=getPublisherProfileUri.d.ts.map