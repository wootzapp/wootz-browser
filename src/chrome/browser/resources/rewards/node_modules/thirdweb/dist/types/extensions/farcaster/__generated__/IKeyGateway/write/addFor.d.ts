import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "addFor" function.
 */
export type AddForParams = WithOverrides<{
    fidOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "fidOwner";
    }>;
    keyType: AbiParameterToPrimitiveType<{
        type: "uint32";
        name: "keyType";
    }>;
    key: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "key";
    }>;
    metadataType: AbiParameterToPrimitiveType<{
        type: "uint8";
        name: "metadataType";
    }>;
    metadata: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "metadata";
    }>;
    deadline: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "deadline";
    }>;
    sig: AbiParameterToPrimitiveType<{
        type: "bytes";
        name: "sig";
    }>;
}>;
export declare const FN_SELECTOR: "0xa005d3d2";
/**
 * Checks if the `addFor` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `addFor` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isAddForSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isAddForSupported(["0x..."]);
 * ```
 */
export declare function isAddForSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "addFor" function.
 * @param options - The options for the addFor function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeAddForParams } from "thirdweb/extensions/farcaster";
 * const result = encodeAddForParams({
 *  fidOwner: ...,
 *  keyType: ...,
 *  key: ...,
 *  metadataType: ...,
 *  metadata: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeAddForParams(options: AddForParams): `0x${string}`;
/**
 * Encodes the "addFor" function into a Hex string with its parameters.
 * @param options - The options for the addFor function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeAddFor } from "thirdweb/extensions/farcaster";
 * const result = encodeAddFor({
 *  fidOwner: ...,
 *  keyType: ...,
 *  key: ...,
 *  metadataType: ...,
 *  metadata: ...,
 *  deadline: ...,
 *  sig: ...,
 * });
 * ```
 */
export declare function encodeAddFor(options: AddForParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "addFor" function on the contract.
 * @param options - The options for the "addFor" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { addFor } from "thirdweb/extensions/farcaster";
 *
 * const transaction = addFor({
 *  contract,
 *  fidOwner: ...,
 *  keyType: ...,
 *  key: ...,
 *  metadataType: ...,
 *  metadata: ...,
 *  deadline: ...,
 *  sig: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function addFor(options: BaseTransactionOptions<AddForParams | {
    asyncParams: () => Promise<AddForParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=addFor.d.ts.map