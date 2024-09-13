import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
/**
 * Represents the parameters for the "state" function.
 */
export type StateParams = {
    proposalId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "proposalId";
    }>;
};
export declare const FN_SELECTOR: "0x3e4f49e6";
/**
 * Checks if the `state` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `state` method is supported.
 * @extension VOTE
 * @example
 * ```ts
 * import { isStateSupported } from "thirdweb/extensions/vote";
 * const supported = isStateSupported(["0x..."]);
 * ```
 */
export declare function isStateSupported(availableSelectors: string[]): boolean;
/**
 * Encodes the parameters for the "state" function.
 * @param options - The options for the state function.
 * @returns The encoded ABI parameters.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeStateParams } from "thirdweb/extensions/vote";
 * const result = encodeStateParams({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeStateParams(options: StateParams): `0x${string}`;
/**
 * Encodes the "state" function into a Hex string with its parameters.
 * @param options - The options for the state function.
 * @returns The encoded hexadecimal string.
 * @extension VOTE
 * @example
 * ```ts
 * import { encodeState } from "thirdweb/extensions/vote";
 * const result = encodeState({
 *  proposalId: ...,
 * });
 * ```
 */
export declare function encodeState(options: StateParams): `${typeof FN_SELECTOR}${string}`;
/**
 * Decodes the result of the state function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension VOTE
 * @example
 * ```ts
 * import { decodeStateResult } from "thirdweb/extensions/vote";
 * const result = decodeStateResultResult("...");
 * ```
 */
export declare function decodeStateResult(result: Hex): number;
/**
 * Calls the "state" function on the contract.
 * @param options - The options for the state function.
 * @returns The parsed result of the function call.
 * @extension VOTE
 * @example
 * ```ts
 * import { state } from "thirdweb/extensions/vote";
 *
 * const result = await state({
 *  contract,
 *  proposalId: ...,
 * });
 *
 * ```
 */
export declare function state(options: BaseTransactionOptions<StateParams>): Promise<number>;
//# sourceMappingURL=state.d.ts.map