import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "postOp" function.
 */
export type PostOpParams = WithOverrides<{
    mode: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "mode";
    }>;
    context: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "context";
    }>;
    actualGasCost: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "actualGasCost";
    }>;
}>;
export declare const FN_SELECTOR: "0xa9a23409";
/**
 * Checks if the `postOp` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `postOp` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isPostOpSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isPostOpSupported(["0x..."]);
 * ```
 */
export declare function isPostOpSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "postOp" function.
 * @param options - The options for the postOp function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodePostOpParams } from "thirdweb/extensions/erc4337";
 * const result = encodePostOpParams({
 *  mode: ...,
 *  context: ...,
 *  actualGasCost: ...,
 * });
 * ```
 */
export declare function encodePostOpParams(options: PostOpParams): `0x${string}`;
/**
 * Encodes the "postOp" function into a Hex string with its parameters.
 * @param options - The options for the postOp function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodePostOp } from "thirdweb/extensions/erc4337";
 * const result = encodePostOp({
 *  mode: ...,
 *  context: ...,
 *  actualGasCost: ...,
 * });
 * ```
 */
export declare function encodePostOp(options: PostOpParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "postOp" function on the contract.
 * @param options - The options for the "postOp" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { postOp } from "thirdweb/extensions/erc4337";
 *
 * const transaction = postOp({
 *  contract,
 *  mode: ...,
 *  context: ...,
 *  actualGasCost: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function postOp(options: BaseTransactionOptions<PostOpParams | {
    asyncParams: () => Promise<PostOpParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=postOp.d.ts.map