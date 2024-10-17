import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "simulateValidation" function.
 */
export type SimulateValidationParams = WithOverrides<{
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
}>;
export declare const FN_SELECTOR: "0xee219423";
/**
 * Checks if the `simulateValidation` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `simulateValidation` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isSimulateValidationSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isSimulateValidationSupported(["0x..."]);
 * ```
 */
export declare function isSimulateValidationSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "simulateValidation" function.
 * @param options - The options for the simulateValidation function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeSimulateValidationParams } from "thirdweb/extensions/erc4337";
 * const result = encodeSimulateValidationParams({
 *  userOp: ...,
 * });
 * ```
 */
export declare function encodeSimulateValidationParams(options: SimulateValidationParams): `0x${string}`;
/**
 * Encodes the "simulateValidation" function into a Hex string with its parameters.
 * @param options - The options for the simulateValidation function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeSimulateValidation } from "thirdweb/extensions/erc4337";
 * const result = encodeSimulateValidation({
 *  userOp: ...,
 * });
 * ```
 */
export declare function encodeSimulateValidation(options: SimulateValidationParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "simulateValidation" function on the contract.
 * @param options - The options for the "simulateValidation" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { simulateValidation } from "thirdweb/extensions/erc4337";
 *
 * const transaction = simulateValidation({
 *  contract,
 *  userOp: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function simulateValidation(options: BaseTransactionOptions<SimulateValidationParams | {
    asyncParams: () => Promise<SimulateValidationParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=simulateValidation.d.ts.map