import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xcb2ef6f7";
/**
 * Checks if the `contractType` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `contractType` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isContractTypeSupported } from "thirdweb/extensions/thirdweb";
 * const supported = isContractTypeSupported(["0x..."]);
 * ```
 */
export declare function isContractTypeSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the contractType function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { decodeContractTypeResult } from "thirdweb/extensions/thirdweb";
 * const result = decodeContractTypeResultResult("...");
 * ```
 */
export declare function decodeContractTypeResult(result: Hex): `0x${string}`;
/**
 * Calls the "contractType" function on the contract.
 * @param options - The options for the contractType function.
 * @returns The parsed result of the function call.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { contractType } from "thirdweb/extensions/thirdweb";
 *
 * const result = await contractType({
 *  contract,
 * });
 *
 * ```
 */
export declare function contractType(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=contractType.d.ts.map