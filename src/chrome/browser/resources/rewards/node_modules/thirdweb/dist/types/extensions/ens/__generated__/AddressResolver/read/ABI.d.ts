import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "ABI" function.
 */
export type ABIParams = {
    name: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "name";
    }>;
    contentTypes: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "contentTypes";
    }>;
};
export declare const FN_SELECTOR: "0x2203ab56";
/**
 * Checks if the `ABI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `ABI` method is supported.
 * @extension ENS
 * @example
 * ```ts
 * import { isABISupported } from "thirdweb/extensions/ens";
 * const supported = isABISupported(["0x..."]);
 * ```
 */
export declare function isABISupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "ABI" function.
 * @param options - The options for the ABI function.
 * @returns The encoded ABI parameters.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeABIParams } from "thirdweb/extensions/ens";
 * const result = encodeABIParams({
 *  name: ...,
 *  contentTypes: ...,
 * });
 * ```
 */
export declare function encodeABIParams(options: ABIParams): `0x${string}`;
/**
 * Encodes the "ABI" function into a Hex string with its parameters.
 * @param options - The options for the ABI function.
 * @returns The encoded hexadecimal string.
 * @extension ENS
 * @example
 * ```ts
 * import { encodeABI } from "thirdweb/extensions/ens";
 * const result = encodeABI({
 *  name: ...,
 *  contentTypes: ...,
 * });
 * ```
 */
export declare function encodeABI(options: ABIParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the ABI function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ENS
 * @example
 * ```ts
 * import { decodeABIResult } from "thirdweb/extensions/ens";
 * const result = decodeABIResultResult("...");
 * ```
 */
export declare function decodeABIResult(result: Hex): readonly [bigint, `0x${string}`];
/**
 * Calls the "ABI" function on the contract.
 * @param options - The options for the ABI function.
 * @returns The parsed result of the function call.
 * @extension ENS
 * @example
 * ```ts
 * import { ABI } from "thirdweb/extensions/ens";
 *
 * const result = await ABI({
 *  contract,
 *  name: ...,
 *  contentTypes: ...,
 * });
 *
 * ```
 */
export declare function ABI(options: BaseTransactionOptions<ABIParams>): Promise<readonly [bigint, `0x${string}`]>;
//# sourceMappingURL=ABI.d.ts.map