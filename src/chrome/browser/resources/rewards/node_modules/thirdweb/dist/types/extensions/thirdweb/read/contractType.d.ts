import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Checks if the `contractType` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `contractType` method is supported.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { isContractTypeSupported } from "thirdweb/extensions/thirdweb";
 *
 * const supported = isContractTypeSupported(["0x..."]);
 * ```
 */
export declare function isContractTypeSupported(availableSelectors: string[]): boolean;
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
export declare function contractType(options: BaseTransactionOptions): Promise<string>;
//# sourceMappingURL=contractType.d.ts.map