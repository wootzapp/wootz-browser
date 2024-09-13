import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "createRuleMultiplicative" function.
 */
export type CreateRuleMultiplicativeParams = WithOverrides<{
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
                name: "scorePerOwnedToken";
            }
        ];
    }>;
}>;
export declare const FN_SELECTOR: "0x1e2e9cb5";
/**
 * Checks if the `createRuleMultiplicative` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `createRuleMultiplicative` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isCreateRuleMultiplicativeSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isCreateRuleMultiplicativeSupported(["0x..."]);
 * ```
 */
export declare function isCreateRuleMultiplicativeSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "createRuleMultiplicative" function.
 * @param options - The options for the createRuleMultiplicative function.
 * @returns The encoded ABI parameters.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeCreateRuleMultiplicativeParams } from "thirdweb/extensions/thirdweb";
 * const result = encodeCreateRuleMultiplicativeParams({
 *  rule: ...,
 * });
 * ```
 */
export declare function encodeCreateRuleMultiplicativeParams(options: CreateRuleMultiplicativeParams): `0x${string}`;
/**
 * Encodes the "createRuleMultiplicative" function into a Hex string with its parameters.
 * @param options - The options for the createRuleMultiplicative function.
 * @returns The encoded hexadecimal string.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { encodeCreateRuleMultiplicative } from "thirdweb/extensions/thirdweb";
 * const result = encodeCreateRuleMultiplicative({
 *  rule: ...,
 * });
 * ```
 */
export declare function encodeCreateRuleMultiplicative(options: CreateRuleMultiplicativeParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "createRuleMultiplicative" function on the contract.
 * @param options - The options for the "createRuleMultiplicative" function.
 * @returns A prepared transaction object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { createRuleMultiplicative } from "thirdweb/extensions/thirdweb";
 *
 * const transaction = createRuleMultiplicative({
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
export declare function createRuleMultiplicative(options: BaseTransactionOptions<CreateRuleMultiplicativeParams | {
    asyncParams: () => Promise<CreateRuleMultiplicativeParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=createRuleMultiplicative.d.ts.map