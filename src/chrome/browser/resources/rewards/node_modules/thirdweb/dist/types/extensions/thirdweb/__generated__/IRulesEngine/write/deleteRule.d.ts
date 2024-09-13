import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "deleteRule" function.
 */
export type DeleteRuleParams = WithOverrides<{
    ruleId: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "ruleId";
    }>;
}>;
export declare const FN_SELECTOR: "0x9d907761";
/**
 * Checks if the `deleteRule` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `deleteRule` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isDeleteRuleSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isDeleteRuleSupported(["0x..."]);
 * ```
 */
export declare function isDeleteRuleSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "deleteRule" function.
 * @param options - The options for the deleteRule function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeDeleteRuleParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeDeleteRuleParams({
 *  ruleId: ...,
 * });
 * ```
 */
export declare function encodeDeleteRuleParams(options: DeleteRuleParams): `0x${string}`;
/**
 * Encodes the "deleteRule" function into a Hex string with its parameters.
 * @param options - The options for the deleteRule function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeDeleteRule } from "thirdweb/extensions/thirdweb";
 * const result = encodeDeleteRule({
 *  ruleId: ...,
 * });
 * ```
 */
export declare function encodeDeleteRule(options: DeleteRuleParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "deleteRule" function on the contract.
 * @param options - The options for the "deleteRule" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { deleteRule } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = deleteRule({
 *  contract,
 *  ruleId: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function deleteRule(options: BaseTransactionOptions<DeleteRuleParams | {
    asyncParams: () => Promise<DeleteRuleParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=deleteRule.d.ts.map