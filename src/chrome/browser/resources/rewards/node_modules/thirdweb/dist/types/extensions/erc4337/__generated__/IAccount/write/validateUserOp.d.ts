import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "validateUserOp" function.
 */
export type ValidateUserOpParams = WithOverrides<{
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
    missingAccountFunds: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "missingAccountFunds";
    }>;
}>;
export declare const FN_SELECTOR: "0x3a871cdd";
/**
 * Checks if the `validateUserOp` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `validateUserOp` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isValidateUserOpSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isValidateUserOpSupported(["0x..."]);
 * ```
 */
export declare function isValidateUserOpSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "validateUserOp" function.
 * @param options - The options for the validateUserOp function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeValidateUserOpParams } from "thirdweb/extensions/erc4337";
 * const result = encodeValidateUserOpParams({
 *  userOp: ...,
 *  userOpHash: ...,
 *  missingAccountFunds: ...,
 * });
 * ```
 */
export declare function encodeValidateUserOpParams(options: ValidateUserOpParams): `0x${string}`;
/**
 * Encodes the "validateUserOp" function into a Hex string with its parameters.
 * @param options - The options for the validateUserOp function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeValidateUserOp } from "thirdweb/extensions/erc4337";
 * const result = encodeValidateUserOp({
 *  userOp: ...,
 *  userOpHash: ...,
 *  missingAccountFunds: ...,
 * });
 * ```
 */
export declare function encodeValidateUserOp(options: ValidateUserOpParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "validateUserOp" function on the contract.
 * @param options - The options for the "validateUserOp" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { validateUserOp } from "thirdweb/extensions/erc4337";
 *
 * const transaction = validateUserOp({
 *  contract,
 *  userOp: ...,
 *  userOpHash: ...,
 *  missingAccountFunds: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function validateUserOp(options: BaseTransactionOptions<ValidateUserOpParams | {
    asyncParams: () => Promise<ValidateUserOpParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=validateUserOp.d.ts.map