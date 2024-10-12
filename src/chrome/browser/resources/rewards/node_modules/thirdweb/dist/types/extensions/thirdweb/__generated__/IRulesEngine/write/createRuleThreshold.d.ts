import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "createRuleThreshold" function.
 */
export type CreateRuleThresholdParams = WithOverrides<{
    rule: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "rule";
        components: [
            {
                type: "address";
                name: "token";
            },
            {
                type: "uint8";
                name: "tokenType";
            },
            {
                type: "uint256";
                name: "tokenId";
            },
            {
                type: "uint256";
                name: "balance";
            },
            {
                type: "uint256";
                name: "score";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x1022a25e";
/**
 * Checks if the `createRuleThreshold` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createRuleThreshold` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isCreateRuleThresholdSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isCreateRuleThresholdSupported(["0x..."]);
 * ```
 */
export declare function isCreateRuleThresholdSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "createRuleThreshold" function.
 * @param options - The options for the createRuleThreshold function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeCreateRuleThresholdParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeCreateRuleThresholdParams({
 *  rule: ...,
 * });
 * ```
 */
export declare function encodeCreateRuleThresholdParams(options: CreateRuleThresholdParams): `0x${string}`;
/**
 * Encodes the "createRuleThreshold" function into a Hex string with its parameters.
 * @param options - The options for the createRuleThreshold function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeCreateRuleThreshold } from "thirdweb/extensions/thirdweb";
 * const result = encodeCreateRuleThreshold({
 *  rule: ...,
 * });
 * ```
 */
export declare function encodeCreateRuleThreshold(options: CreateRuleThresholdParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "createRuleThreshold" function on the contract.
 * @param options - The options for the "createRuleThreshold" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { createRuleThreshold } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = createRuleThreshold({
 *  contract,
 *  rule: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function createRuleThreshold(options: BaseTransactionOptions<CreateRuleThresholdParams | {
    asyncParams: () => Promise<CreateRuleThresholdParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=createRuleThreshold.d.ts.map