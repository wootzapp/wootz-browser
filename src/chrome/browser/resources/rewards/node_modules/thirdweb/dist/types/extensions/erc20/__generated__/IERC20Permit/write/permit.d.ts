import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "permit" function.
 */
export type PermitParams = WithOverrides<{
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
    }>;
    spender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "spender";
    }>;
    value: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "value";
    }>;
    deadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "deadline";
    }>;
    v: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "v";
    }>;
    r: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "r";
    }>;
    s: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "s";
    }>;
}>;
export declare const FN_SELECTOR: "0xd505accf";
/**
 * Checks if the `permit` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `permit` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isPermitSupported } from "thirdweb/extensions/erc20";
 *
 * const supported = isPermitSupported(["0x..."]);
 * ```
 */
export declare function isPermitSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "permit" function.
 * @param options - The options for the permit function.
 * @returns The encoded ABI parameters.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodePermitParams } from "thirdweb/extensions/erc20";
 * const result = encodePermitParams({
 *  owner: ...,
 *  spender: ...,
 *  value: ...,
 *  deadline: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodePermitParams(options: PermitParams): `0x${string}`;
/**
 * Encodes the "permit" function into a Hex string with its parameters.
 * @param options - The options for the permit function.
 * @returns The encoded hexadecimal string.
 * @extension ERC20
 * @example
 * ```ts
 * import { encodePermit } from "thirdweb/extensions/erc20";
 * const result = encodePermit({
 *  owner: ...,
 *  spender: ...,
 *  value: ...,
 *  deadline: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 * });
 * ```
 */
export declare function encodePermit(options: PermitParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "permit" function on the contract.
 * @param options - The options for the "permit" function.
 * @returns A prepared transaction object.
 * @extension ERC20
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { permit } from "thirdweb/extensions/erc20";
 *
 * const transaction = permit({
 *  contract,
 *  owner: ...,
 *  spender: ...,
 *  value: ...,
 *  deadline: ...,
 *  v: ...,
 *  r: ...,
 *  s: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function permit(options: BaseTransactionOptions<PermitParams | {
    asyncParams: () => Promise<PermitParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=permit.d.ts.map