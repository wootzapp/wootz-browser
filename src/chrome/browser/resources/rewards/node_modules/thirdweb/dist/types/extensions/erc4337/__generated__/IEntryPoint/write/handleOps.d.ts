import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "handleOps" function.
 */
export type HandleOpsParams = WithOverrides<{
    ops: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "ops";
        components: [
            {
                type: "address";
                name: "sender";
            },
            {
                type: "uint256";
                name: "nonce";
            },
            {
                type: "bytes";
                name: "initCode";
            },
            {
                type: "bytes";
                name: "callData";
            },
            {
                type: "uint256";
                name: "callGasLimit";
            },
            {
                type: "uint256";
                name: "verificationGasLimit";
            },
            {
                type: "uint256";
                name: "preVerificationGas";
            },
            {
                type: "uint256";
                name: "maxFeePerGas";
            },
            {
                type: "uint256";
                name: "maxPriorityFeePerGas";
            },
            {
                type: "bytes";
                name: "paymasterAndData";
            },
            {
                type: "bytes";
                name: "signature";
            }
        ];
    }>;
    beneficiary: AbiParameterToPrimitiveType<{
        type: "address";
        name: "beneficiary";
    }>;
}>;
export declare const FN_SELECTOR: "0x1fad948c";
/**
 * Checks if the `handleOps` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `handleOps` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isHandleOpsSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isHandleOpsSupported(["0x..."]);
 * ```
 */
export declare function isHandleOpsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "handleOps" function.
 * @param options - The options for the handleOps function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeHandleOpsParams } from "thirdweb/extensions/erc4337";
 * const result = encodeHandleOpsParams({
 *  ops: ...,
 *  beneficiary: ...,
 * });
 * ```
 */
export declare function encodeHandleOpsParams(options: HandleOpsParams): `0x${string}`;
/**
 * Encodes the "handleOps" function into a Hex string with its parameters.
 * @param options - The options for the handleOps function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeHandleOps } from "thirdweb/extensions/erc4337";
 * const result = encodeHandleOps({
 *  ops: ...,
 *  beneficiary: ...,
 * });
 * ```
 */
export declare function encodeHandleOps(options: HandleOpsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "handleOps" function on the contract.
 * @param options - The options for the "handleOps" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { handleOps } from "thirdweb/extensions/erc4337";
 *
 * const transaction = handleOps({
 *  contract,
 *  ops: ...,
 *  beneficiary: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function handleOps(options: BaseTransactionOptions<HandleOpsParams | {
    asyncParams: () => Promise<HandleOpsParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=handleOps.d.ts.map