import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "simulateHandleOp" function.
 */
export type SimulateHandleOpParams = WithOverrides<{
    op: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "op";
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
    target: AbiParameterToPrimitiveType<{
        type: "address";
        name: "target";
    }>;
    targetCallData: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "targetCallData";
    }>;
}>;
export declare const FN_SELECTOR: "0xd6383f94";
/**
 * Checks if the `simulateHandleOp` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `simulateHandleOp` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isSimulateHandleOpSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isSimulateHandleOpSupported(["0x..."]);
 * ```
 */
export declare function isSimulateHandleOpSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "simulateHandleOp" function.
 * @param options - The options for the simulateHandleOp function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeSimulateHandleOpParams } from "thirdweb/extensions/erc4337";
 * const result = encodeSimulateHandleOpParams({
 *  op: ...,
 *  target: ...,
 *  targetCallData: ...,
 * });
 * ```
 */
export declare function encodeSimulateHandleOpParams(options: SimulateHandleOpParams): `0x${string}`;
/**
 * Encodes the "simulateHandleOp" function into a Hex string with its parameters.
 * @param options - The options for the simulateHandleOp function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeSimulateHandleOp } from "thirdweb/extensions/erc4337";
 * const result = encodeSimulateHandleOp({
 *  op: ...,
 *  target: ...,
 *  targetCallData: ...,
 * });
 * ```
 */
export declare function encodeSimulateHandleOp(options: SimulateHandleOpParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "simulateHandleOp" function on the contract.
 * @param options - The options for the "simulateHandleOp" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { simulateHandleOp } from "thirdweb/extensions/erc4337";
 *
 * const transaction = simulateHandleOp({
 *  contract,
 *  op: ...,
 *  target: ...,
 *  targetCallData: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function simulateHandleOp(options: BaseTransactionOptions<SimulateHandleOpParams | {
    asyncParams: () => Promise<SimulateHandleOpParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=simulateHandleOp.d.ts.map