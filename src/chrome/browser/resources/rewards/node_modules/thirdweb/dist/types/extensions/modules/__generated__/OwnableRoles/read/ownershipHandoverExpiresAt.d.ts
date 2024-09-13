import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "ownershipHandoverExpiresAt" function.
 */
export type OwnershipHandoverExpiresAtParams = {
    pendingOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "pendingOwner";
    }>;
};
export declare const FN_SELECTOR: "0xfee81cf4";
/**
 * Checks if the `ownershipHandoverExpiresAt` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `ownershipHandoverExpiresAt` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isOwnershipHandoverExpiresAtSupported } from "thirdweb/extensions/modules";
 * const supported = isOwnershipHandoverExpiresAtSupported(["0x..."]);
 * ```
 */
export declare function isOwnershipHandoverExpiresAtSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "ownershipHandoverExpiresAt" function.
 * @param options - The options for the ownershipHandoverExpiresAt function.
 * @returns The encoded ABI parameters.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeOwnershipHandoverExpiresAtParams } from "thirdweb/extensions/modules";
 * const result = encodeOwnershipHandoverExpiresAtParams({
 *  pendingOwner: ...,
 * });
 * ```
 */
export declare function encodeOwnershipHandoverExpiresAtParams(options: OwnershipHandoverExpiresAtParams): `0x${string}`;
/**
 * Encodes the "ownershipHandoverExpiresAt" function into a Hex string with its parameters.
 * @param options - The options for the ownershipHandoverExpiresAt function.
 * @returns The encoded hexadecimal string.
 * @extension MODULES
 * @example
 * ```ts
 * import { encodeOwnershipHandoverExpiresAt } from "thirdweb/extensions/modules";
 * const result = encodeOwnershipHandoverExpiresAt({
 *  pendingOwner: ...,
 * });
 * ```
 */
export declare function encodeOwnershipHandoverExpiresAt(options: OwnershipHandoverExpiresAtParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the ownershipHandoverExpiresAt function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MODULES
 * @example
 * ```ts
 * import { decodeOwnershipHandoverExpiresAtResult } from "thirdweb/extensions/modules";
 * const result = decodeOwnershipHandoverExpiresAtResultResult("...");
 * ```
 */
export declare function decodeOwnershipHandoverExpiresAtResult(result: Hex): bigint;
/**
 * Calls the "ownershipHandoverExpiresAt" function on the contract.
 * @param options - The options for the ownershipHandoverExpiresAt function.
 * @returns The parsed result of the function call.
 * @extension MODULES
 * @example
 * ```ts
 * import { ownershipHandoverExpiresAt } from "thirdweb/extensions/modules";
 *
 * const result = await ownershipHandoverExpiresAt({
 *  contract,
 *  pendingOwner: ...,
 * });
 *
 * ```
 */
export declare function ownershipHandoverExpiresAt(options: BaseTransactionOptions<OwnershipHandoverExpiresAtParams>): Promise<bigint>;
//# sourceMappingURL=ownershipHandoverExpiresAt.d.ts.map