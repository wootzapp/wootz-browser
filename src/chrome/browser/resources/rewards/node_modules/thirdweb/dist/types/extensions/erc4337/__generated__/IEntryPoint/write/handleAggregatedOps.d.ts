import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "handleAggregatedOps" function.
 */
export type HandleAggregatedOpsParams = WithOverrides<{
    opsPerAggregator: AbiParameterToPrimitiveType<{
        type: "tuple[]";
        name: "opsPerAggregator";
        components: [
            {
                type: "tuple[]";
                name: "userOps";
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
            },
            {
                type: "address";
                name: "aggregator";
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
export declare const FN_SELECTOR: "0x4b1d7cf5";
/**
 * Checks if the `handleAggregatedOps` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `handleAggregatedOps` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isHandleAggregatedOpsSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isHandleAggregatedOpsSupported(["0x..."]);
 * ```
 */
export declare function isHandleAggregatedOpsSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "handleAggregatedOps" function.
 * @param options - The options for the handleAggregatedOps function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeHandleAggregatedOpsParams } from "thirdweb/extensions/erc4337";
 * const result = encodeHandleAggregatedOpsParams({
 *  opsPerAggregator: ...,
 *  beneficiary: ...,
 * });
 * ```
 */
export declare function encodeHandleAggregatedOpsParams(options: HandleAggregatedOpsParams): `0x${string}`;
/**
 * Encodes the "handleAggregatedOps" function into a Hex string with its parameters.
 * @param options - The options for the handleAggregatedOps function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeHandleAggregatedOps } from "thirdweb/extensions/erc4337";
 * const result = encodeHandleAggregatedOps({
 *  opsPerAggregator: ...,
 *  beneficiary: ...,
 * });
 * ```
 */
export declare function encodeHandleAggregatedOps(options: HandleAggregatedOpsParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "handleAggregatedOps" function on the contract.
 * @param options - The options for the "handleAggregatedOps" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { handleAggregatedOps } from "thirdweb/extensions/erc4337";
 *
 * const transaction = handleAggregatedOps({
 *  contract,
 *  opsPerAggregator: ...,
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
export declare function handleAggregatedOps(options: BaseTransactionOptions<HandleAggregatedOpsParams | {
    asyncParams: () => Promise<HandleAggregatedOpsParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=handleAggregatedOps.d.ts.map