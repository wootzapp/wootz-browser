import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xd5bac7f3";
/**
 * Checks if the `CHANGE_RECOVERY_ADDRESS_TYPEHASH` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `CHANGE_RECOVERY_ADDRESS_TYPEHASH` method is supported.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { isCHANGE_RECOVERY_ADDRESS_TYPEHASHSupported } from "thirdweb/extensions/farcaster";
 * const supported = isCHANGE_RECOVERY_ADDRESS_TYPEHASHSupported(["0x..."]);
 * ```
 */
export declare function isCHANGE_RECOVERY_ADDRESS_TYPEHASHSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the CHANGE_RECOVERY_ADDRESS_TYPEHASH function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { decodeCHANGE_RECOVERY_ADDRESS_TYPEHASHResult } from "thirdweb/extensions/farcaster";
 * const result = decodeCHANGE_RECOVERY_ADDRESS_TYPEHASHResultResult("...");
 * ```
 */
export declare function decodeCHANGE_RECOVERY_ADDRESS_TYPEHASHResult(result: Hex): `0x${string}`;
/**
 * Calls the "CHANGE_RECOVERY_ADDRESS_TYPEHASH" function on the contract.
 * @param options - The options for the CHANGE_RECOVERY_ADDRESS_TYPEHASH function.
 * @returns The parsed result of the function call.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { CHANGE_RECOVERY_ADDRESS_TYPEHASH } from "thirdweb/extensions/farcaster";
 *
 * const result = await CHANGE_RECOVERY_ADDRESS_TYPEHASH({
 *  contract,
 * });
 *
 * ```
 */
export declare function CHANGE_RECOVERY_ADDRESS_TYPEHASH(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=CHANGE_RECOVERY_ADDRESS_TYPEHASH.d.ts.map