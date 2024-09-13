import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "setMerkleRoot" function.
 */
export type SetMerkleRootParams = WithOverrides<{
    token: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_token";
    }>;
    tokenMerkleRoot: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "_tokenMerkleRoot";
    }>;
    resetClaimStatus: AbiParameterToPrimitiveType<{
        type: "bool";
        name: "_resetClaimStatus";
    }>;
}>;
export declare const FN_SELECTOR: "0x8259a87b";
/**
 * Checks if the `setMerkleRoot` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `setMerkleRoot` method is supported.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { isSetMerkleRootSupported } from "thirdweb/extensions/airdrop";
 *
 * const supported = isSetMerkleRootSupported(["0x..."]);
 * ```
 */
export declare function isSetMerkleRootSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "setMerkleRoot" function.
 * @param options - The options for the setMerkleRoot function.
 * @returns The encoded ABI parameters.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeSetMerkleRootParams } from "thirdweb/extensions/airdrop";
 * const result = encodeSetMerkleRootParams({
 *  token: ...,
 *  tokenMerkleRoot: ...,
 *  resetClaimStatus: ...,
 * });
 * ```
 */
export declare function encodeSetMerkleRootParams(options: SetMerkleRootParams): `0x${string}`;
/**
 * Encodes the "setMerkleRoot" function into a Hex string with its parameters.
 * @param options - The options for the setMerkleRoot function.
 * @returns The encoded hexadecimal string.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { encodeSetMerkleRoot } from "thirdweb/extensions/airdrop";
 * const result = encodeSetMerkleRoot({
 *  token: ...,
 *  tokenMerkleRoot: ...,
 *  resetClaimStatus: ...,
 * });
 * ```
 */
export declare function encodeSetMerkleRoot(options: SetMerkleRootParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "setMerkleRoot" function on the contract.
 * @param options - The options for the "setMerkleRoot" function.
 * @returns A prepared transaction object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { setMerkleRoot } from "thirdweb/extensions/airdrop";
 *
 * const transaction = setMerkleRoot({
 *  contract,
 *  token: ...,
 *  tokenMerkleRoot: ...,
 *  resetClaimStatus: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function setMerkleRoot(options: BaseTransactionOptions<SetMerkleRootParams | {
    asyncParams: () => Promise<SetMerkleRootParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setMerkleRoot.d.ts.map