import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x3644e515";
/**
 * Checks if the `DOMAIN_SEPARATOR` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `DOMAIN_SEPARATOR` method is supported.
 * @extension ERC20
 * @example
 * ```ts
 * import { isDOMAIN_SEPARATORSupported } from "thirdweb/extensions/erc20";
 * const supported = isDOMAIN_SEPARATORSupported(["0x..."]);
 * ```
 */
export declare function isDOMAIN_SEPARATORSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the DOMAIN_SEPARATOR function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension ERC20
 * @example
 * ```ts
 * import { decodeDOMAIN_SEPARATORResult } from "thirdweb/extensions/erc20";
 * const result = decodeDOMAIN_SEPARATORResultResult("...");
 * ```
 */
export declare function decodeDOMAIN_SEPARATORResult(result: Hex): `0x${string}`;
/**
 * Calls the "DOMAIN_SEPARATOR" function on the contract.
 * @param options - The options for the DOMAIN_SEPARATOR function.
 * @returns The parsed result of the function call.
 * @extension ERC20
 * @example
 * ```ts
 * import { DOMAIN_SEPARATOR } from "thirdweb/extensions/erc20";
 *
 * const result = await DOMAIN_SEPARATOR({
 *  contract,
 * });
 *
 * ```
 */
export declare function DOMAIN_SEPARATOR(options: BaseTransactionOptions): Promise<`0x${string}`>;
//# sourceMappingURL=DOMAIN_SEPARATOR.d.ts.map