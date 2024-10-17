import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "isTransferEnabledFor" function.
 */
export type IsTransferEnabledForParams = {
    target: AbiParameterToPrimitiveType<{
        type: "address";
        name: "target";
    }>;
};
export declare const FN_SELECTOR: "0x735d0538";
/**
 * Checks if the `isTransferEnabledFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isTransferEnabledFor` method is supported.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const supported = TransferableERC20.isIsTransferEnabledForSupported(["0x..."]);
 * ```
 */
export declare function isIsTransferEnabledForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "isTransferEnabledFor" function.
 * @param options - The options for the isTransferEnabledFor function.
 * @returns The encoded ABI parameters.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.encodeIsTransferEnabledForParams({
 *  target: ...,
 * });
 * ```
 */
export declare function encodeIsTransferEnabledForParams(options: IsTransferEnabledForParams): `0x${string}`;
/**
 * Encodes the "isTransferEnabledFor" function into a Hex string with its parameters.
 * @param options - The options for the isTransferEnabledFor function.
 * @returns The encoded hexadecimal string.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.encodeIsTransferEnabledFor({
 *  target: ...,
 * });
 * ```
 */
export declare function encodeIsTransferEnabledFor(options: IsTransferEnabledForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the isTransferEnabledFor function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 * const result = TransferableERC20.decodeIsTransferEnabledForResultResult("...");
 * ```
 */
export declare function decodeIsTransferEnabledForResult(result: Hex): boolean;
/**
 * Calls the "isTransferEnabledFor" function on the contract.
 * @param options - The options for the isTransferEnabledFor function.
 * @returns The parsed result of the function call.
 * @modules TransferableERC20
 * @example
 * ```ts
 * import { TransferableERC20 } from "thirdweb/modules";
 *
 * const result = await TransferableERC20.isTransferEnabledFor({
 *  contract,
 *  target: ...,
 * });
 *
 * ```
 */
export declare function isTransferEnabledFor(options: BaseTransactionOptions<IsTransferEnabledForParams>): Promise<boolean>;
//# sourceMappingURL=isTransferEnabledFor.d.ts.map