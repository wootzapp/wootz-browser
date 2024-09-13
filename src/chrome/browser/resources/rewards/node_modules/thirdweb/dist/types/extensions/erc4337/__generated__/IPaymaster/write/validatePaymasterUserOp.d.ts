import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "validatePaymasterUserOp" function.
 */
export type ValidatePaymasterUserOpParams = WithOverrides<{
    userOp: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "userOp";
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
    userOpHash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "userOpHash";
    }>;
    maxCost: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "maxCost";
    }>;
}>;
export declare const FN_SELECTOR: "0xf465c77e";
/**
 * Checks if the `validatePaymasterUserOp` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `validatePaymasterUserOp` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isValidatePaymasterUserOpSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isValidatePaymasterUserOpSupported(["0x..."]);
 * ```
 */
export declare function isValidatePaymasterUserOpSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "validatePaymasterUserOp" function.
 * @param options - The options for the validatePaymasterUserOp function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeValidatePaymasterUserOpParams } from "thirdweb/extensions/erc4337";
 * const result = encodeValidatePaymasterUserOpParams({
 *  userOp: ...,
 *  userOpHash: ...,
 *  maxCost: ...,
 * });
 * ```
 */
export declare function encodeValidatePaymasterUserOpParams(options: ValidatePaymasterUserOpParams): `0x${string}`;
/**
 * Encodes the "validatePaymasterUserOp" function into a Hex string with its parameters.
 * @param options - The options for the validatePaymasterUserOp function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeValidatePaymasterUserOp } from "thirdweb/extensions/erc4337";
 * const result = encodeValidatePaymasterUserOp({
 *  userOp: ...,
 *  userOpHash: ...,
 *  maxCost: ...,
 * });
 * ```
 */
export declare function encodeValidatePaymasterUserOp(options: ValidatePaymasterUserOpParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "validatePaymasterUserOp" function on the contract.
 * @param options - The options for the "validatePaymasterUserOp" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { validatePaymasterUserOp } from "thirdweb/extensions/erc4337";
 *
 * const transaction = validatePaymasterUserOp({
 *  contract,
 *  userOp: ...,
 *  userOpHash: ...,
 *  maxCost: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function validatePaymasterUserOp(options: BaseTransactionOptions<ValidatePaymasterUserOpParams | {
    asyncParams: () => Promise<ValidatePaymasterUserOpParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=validatePaymasterUserOp.d.ts.map