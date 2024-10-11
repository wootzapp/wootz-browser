import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0xe8a3d485";
/**
 * Checks if the `contractURI` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `contractURI` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isContractURISupported } from "thirdweb/extensions/marketplace";
 * const supported = isContractURISupported(["0x..."]);
 * ```
 */
export declare function isContractURISupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the contractURI function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { decodeContractURIResult } from "thirdweb/extensions/marketplace";
 * const result = decodeContractURIResultResult("...");
 * ```
 */
export declare function decodeContractURIResult(result: Hex): string;
/**
 * Calls the "contractURI" function on the contract.
 * @param options - The options for the contractURI function.
 * @returns The parsed result of the function call.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { contractURI } from "thirdweb/extensions/marketplace";
 *
 * const result = await contractURI({
 *  contract,
 * });
 *
 * ```
 */
export declare function contractURI(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=contractURI.d.ts.map