import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setPermissionsForSigner" function.
 */
export type SetPermissionsForSignerParams = WithOverrides<{
    req: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "req";
        components: [
            {
                type: "address";
                name: "signer";
            },
            {
                type: "uint8";
                name: "isAdmin";
            },
            {
                type: "address[]";
                name: "approvedTargets";
            },
            {
                type: "uint256";
                name: "nativeTokenLimitPerTransaction";
            },
            {
                type: "uint128";
                name: "permissionStartTimestamp";
            },
            {
                type: "uint128";
                name: "permissionEndTimestamp";
            },
            {
                type: "uint128";
                name: "reqValidityStartTimestamp";
            },
            {
                type: "uint128";
                name: "reqValidityEndTimestamp";
            },
            {
                type: "bytes32";
                name: "uid";
            }
        ];
    }>;
    signature: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "signature";
    }>;
}>;
export declare const FN_SELECTOR: "0x5892e236";
/**
 * Checks if the `setPermissionsForSigner` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setPermissionsForSigner` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isSetPermissionsForSignerSupported } from "thirdweb/extensions/erc4337";
 *
 * const supported = isSetPermissionsForSignerSupported(["0x..."]);
 * ```
 */
export declare function isSetPermissionsForSignerSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setPermissionsForSigner" function.
 * @param options - The options for the setPermissionsForSigner function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeSetPermissionsForSignerParams } from "thirdweb/extensions/erc4337";
 * const result = encodeSetPermissionsForSignerParams({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeSetPermissionsForSignerParams(options: SetPermissionsForSignerParams): `0x${string}`;
/**
 * Encodes the "setPermissionsForSigner" function into a Hex string with its parameters.
 * @param options - The options for the setPermissionsForSigner function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeSetPermissionsForSigner } from "thirdweb/extensions/erc4337";
 * const result = encodeSetPermissionsForSigner({
 *  req: ...,
 *  signature: ...,
 * });
 * ```
 */
export declare function encodeSetPermissionsForSigner(options: SetPermissionsForSignerParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setPermissionsForSigner" function on the contract.
 * @param options - The options for the "setPermissionsForSigner" function.
 * @returns A prepared transaction object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setPermissionsForSigner } from "thirdweb/extensions/erc4337";
 *
 * const transaction = setPermissionsForSigner({
 *  contract,
 *  req: ...,
 *  signature: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setPermissionsForSigner(options: BaseTransactionOptions<SetPermissionsForSignerParams | {
    asyncParams: () => Promise<SetPermissionsForSignerParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setPermissionsForSigner.d.ts.map