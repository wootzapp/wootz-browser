import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "getDepositInfo" function.
 */
export type GetDepositInfoParams = {
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
    }>;
};
export declare const FN_SELECTOR: "0x5287ce12";
/**
 * Checks if the `getDepositInfo` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getDepositInfo` method is supported.
 * @extension ERC4337
 * @example
 * ```ts
 * import { isGetDepositInfoSupported } from "thirdweb/extensions/erc4337";
 * const supported = isGetDepositInfoSupported(["0x..."]);
 * ```
 */
export declare function isGetDepositInfoSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "getDepositInfo" function.
 * @param options - The options for the getDepositInfo function.
 * @returns The encoded ABI parameters.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetDepositInfoParams } from "thirdweb/extensions/erc4337";
 * const result = encodeGetDepositInfoParams({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeGetDepositInfoParams(options: GetDepositInfoParams): `0x${string}`;
/**
 * Encodes the "getDepositInfo" function into a Hex string with its parameters.
 * @param options - The options for the getDepositInfo function.
 * @returns The encoded hexadecimal string.
 * @extension ERC4337
 * @example
 * ```ts
 * import { encodeGetDepositInfo } from "thirdweb/extensions/erc4337";
 * const result = encodeGetDepositInfo({
 *  account: ...,
 * });
 * ```
 */
export declare function encodeGetDepositInfo(options: GetDepositInfoParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the getDepositInfo function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC4337
 * @example
 * ```ts
 * import { decodeGetDepositInfoResult } from "thirdweb/extensions/erc4337";
 * const result = decodeGetDepositInfoResultResult("...");
 * ```
 */
export declare function decodeGetDepositInfoResult(result: Hex): {
    deposit: bigint;
    staked: boolean;
    stake: bigint;
    unstakeDelaySec: number;
    withdrawTime: number;
};
/**
 * Calls the "getDepositInfo" function on the contract.
 * @param options - The options for the getDepositInfo function.
 * @returns The parsed result of the function call.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getDepositInfo } from "thirdweb/extensions/erc4337";
 *
 * const result = await getDepositInfo({
 *  contract,
 *  account: ...,
 * });
 *
 * ```
 */
export declare function getDepositInfo(options: BaseTransactionOptions<GetDepositInfoParams>): Promise<{
    deposit: bigint;
    staked: boolean;
    stake: bigint;
    unstakeDelaySec: number;
    withdrawTime: number;
}>;
//# sourceMappingURL=getDepositInfo.d.ts.map