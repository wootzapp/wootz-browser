import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getUserOpHash" function.
 */
export type GetUserOpHashParams = {
    userOp: AbiParameterToPrimitiveType<{
        type: "tuple";
        name: "userOp";
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
                type: "bytes32";
                name: "accountGasLimits";
            },
            {
                type: "uint256";
                name: "preVerificationGas";
            },
            {
                type: "bytes32";
                name: "gasFees";
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
    }>;
};
export declare const FN_SELECTOR: "0x22cdde4c";
/**
 * Checks if the `getUserOpHash` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getUserOpHash` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isGetUserOpHashSupported } from "thirdweb/extensions/erc4337";
 * const supported = isGetUserOpHashSupported(["0x..."]);
 * ```
 */
export declare function isGetUserOpHashSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getUserOpHash" function.
 * @param options - The options for the getUserOpHash function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetUserOpHashParams } from "thirdweb/extensions/erc4337";
 * const result = encodeGetUserOpHashParams({
 *  userOp: ...,
 * });
 * ```
 */
export declare function encodeGetUserOpHashParams(options: GetUserOpHashParams): `0x${string}`;
/**
 * Encodes the "getUserOpHash" function into a Hex string with its parameters.
 * @param options - The options for the getUserOpHash function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetUserOpHash } from "thirdweb/extensions/erc4337";
 * const result = encodeGetUserOpHash({
 *  userOp: ...,
 * });
 * ```
 */
export declare function encodeGetUserOpHash(options: GetUserOpHashParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getUserOpHash function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4337
 * @example
 * ```ts
 * import { decodeGetUserOpHashResult } from "thirdweb/extensions/erc4337";
 * const result = decodeGetUserOpHashResultResult("...");
 * ```
 */
export declare function decodeGetUserOpHashResult(result: Hex): `0x${string}`;
/**
 * Calls the "getUserOpHash" function on the contract.
 * @param options - The options for the getUserOpHash function.
 * @returns The parsed result of the function call.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getUserOpHash } from "thirdweb/extensions/erc4337";
 *
 * const result = await getUserOpHash({
 *  contract,
 *  userOp: ...,
 * });
 *
 * ```
 */
export declare function getUserOpHash(options: BaseTransactionOptions<GetUserOpHashParams>): Promise<`0x${string}`>;
//# sourceMappingURL=getUserOpHash.d.ts.map