import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "mint" function.
 */
export type MintParams = WithOverrides<{
    shares: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "shares";
    }>;
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
    }>;
}>;
export declare const FN_SELECTOR: "0x94bf804d";
/**
 * Checks if the `mint` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `mint` method is supported.
 * @extension ERC4626
 * @example
 * ```ts
 * import { isMintSupported } from "thirdweb/extensions/erc4626";
 *
 * const supported = isMintSupported(["0x..."]);
 * ```
 */
export declare function isMintSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "mint" function.
 * @param options - The options for the mint function.
 * @returns The encoded ABI parameters.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMintParams } from "thirdweb/extensions/erc4626";
 * const result = encodeMintParams({
 *  shares: ...,
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeMintParams(options: MintParams): `0x${string}`;
/**
 * Encodes the "mint" function into a Hex string with its parameters.
 * @param options - The options for the mint function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4626
 * @example
 * ```ts
 * import { encodeMint } from "thirdweb/extensions/erc4626";
 * const result = encodeMint({
 *  shares: ...,
 *  receiver: ...,
 * });
 * ```
 */
export declare function encodeMint(options: MintParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "mint" function on the contract.
 * @param options - The options for the "mint" function.
 * @returns A prepared transaction object.
 * @extension ERC4626
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { mint } from "thirdweb/extensions/erc4626";
 *
 * const transaction = mint({
 *  contract,
 *  shares: ...,
 *  receiver: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function mint(options: BaseTransactionOptions<MintParams | {
    asyncParams: () => Promise<MintParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=mint.d.ts.map