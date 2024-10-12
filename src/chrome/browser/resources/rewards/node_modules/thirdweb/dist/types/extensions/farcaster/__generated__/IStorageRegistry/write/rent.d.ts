import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions, WithOverrides } from "../../../../../transaction/types.js";
/**
 * Represents the parameters for the "rent" function.
 */
export type RentParams = WithOverrides<{
    fid: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fid";
    }>;
    units: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "units";
    }>;
}>;
export declare const FN_SELECTOR: "0x783a112b";
/**
 * Checks if the `rent` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `rent` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isRentSupported } from "thirdweb/extensions/farcaster";
 *
 * const supported = isRentSupported(["0x..."]);
 * ```
 */
export declare function isRentSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "rent" function.
 * @param options - The options for the rent function.
 * @returns The encoded ABI parameters.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRentParams } from "thirdweb/extensions/farcaster";
 * const result = encodeRentParams({
 *  fid: ...,
 *  units: ...,
 * });
 * ```
 */
export declare function encodeRentParams(options: RentParams): `0x${string}`;
/**
 * Encodes the "rent" function into a Hex string with its parameters.
 * @param options - The options for the rent function.
 * @returns The encoded hexadecimal string.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { encodeRent } from "thirdweb/extensions/farcaster";
 * const result = encodeRent({
 *  fid: ...,
 *  units: ...,
 * });
 * ```
 */
export declare function encodeRent(options: RentParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Prepares a transaction to call the "rent" function on the contract.
 * @param options - The options for the "rent" function.
 * @returns A prepared transaction object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { sendTransaction } from "thirdweb";
 * import { rent } from "thirdweb/extensions/farcaster";
 *
 * const transaction = rent({
 *  contract,
 *  fid: ...,
 *  units: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function rent(options: BaseTransactionOptions<RentParams | {
    asyncParams: () => Promise<RentParams>;
}>): import("../../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=rent.d.ts.map