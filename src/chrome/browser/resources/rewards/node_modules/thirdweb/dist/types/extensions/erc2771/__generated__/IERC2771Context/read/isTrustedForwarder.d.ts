import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isTrustedForwarder" function.
 */
export type IsTrustedForwarderParams = {
    forwarder: AbiParameterToPrimitiveType<{
        type: "address";
        name: "forwarder";
    }>;
};
export declare const FN_SELECTOR: "0x572b6c05";
/**
 * Checks if the `isTrustedForwarder` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isTrustedForwarder` method is supported.
 * @extension ERC2771
 * @example
 * ```ts
 * import { isIsTrustedForwarderSupported } from "thirdweb/extensions/erc2771";
 * const supported = isIsTrustedForwarderSupported(["0x..."]);
 * ```
 */
export declare function isIsTrustedForwarderSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isTrustedForwarder" function.
 * @param options - The options for the isTrustedForwarder function.
 * @returns The encoded ABI parameters.
 * @extension ERC2771
 * @example
 * ```ts
 * import { encodeIsTrustedForwarderParams } from "thirdweb/extensions/erc2771";
 * const result = encodeIsTrustedForwarderParams({
 *  forwarder: ...,
 * });
 * ```
 */
export declare function encodeIsTrustedForwarderParams(options: IsTrustedForwarderParams): `0x${string}`;
/**
 * Encodes the "isTrustedForwarder" function into a Hex string with its parameters.
 * @param options - The options for the isTrustedForwarder function.
 * @returns The encoded hexadecimal string.
 * @extension ERC2771
 * @example
 * ```ts
 * import { encodeIsTrustedForwarder } from "thirdweb/extensions/erc2771";
 * const result = encodeIsTrustedForwarder({
 *  forwarder: ...,
 * });
 * ```
 */
export declare function encodeIsTrustedForwarder(options: IsTrustedForwarderParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isTrustedForwarder function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC2771
 * @example
 * ```ts
 * import { decodeIsTrustedForwarderResult } from "thirdweb/extensions/erc2771";
 * const result = decodeIsTrustedForwarderResultResult("...");
 * ```
 */
export declare function decodeIsTrustedForwarderResult(result: Hex): boolean;
/**
 * Calls the "isTrustedForwarder" function on the contract.
 * @param options - The options for the isTrustedForwarder function.
 * @returns The parsed result of the function call.
 * @extension ERC2771
 * @example
 * ```ts
 * import { isTrustedForwarder } from "thirdweb/extensions/erc2771";
 *
 * const result = await isTrustedForwarder({
 *  contract,
 *  forwarder: ...,
 * });
 *
 * ```
 */
export declare function isTrustedForwarder(options: BaseTransactionOptions<IsTrustedForwarderParams>): Promise<boolean>;
//# sourceMappingURL=isTrustedForwarder.d.ts.map