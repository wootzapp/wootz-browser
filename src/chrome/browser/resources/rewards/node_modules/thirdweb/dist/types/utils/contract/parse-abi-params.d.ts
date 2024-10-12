/**
 * Converts an array of parameter values to their respective types based on the provided type array.
 *
 * This utility function is particularly useful for ensuring that parameter values are correctly formatted
 * according to the expected types before they are used in further processing or passed to a Solidity smart contract.
 *
 * @param {string[]} constructorParamTypes - An array of type strings indicating the expected types of the values,
 *                   following Solidity type conventions (e.g., "address", "uint256", "bool").
 * @param {unknown[]} constructorParamValues - An array of values to be converted according to the types.
 * @returns - An array of values converted to their respective types.
 *
 * @example
 * ```ts
 * import { parseAbiParams } from "thirdweb/utils";
 *
 * const example1 = parseAbiParams(
 *   ["address", "uint256"],
 *   ["0x.....", "1200000"]
 * ); // result: ["0x......", 1200000n]
 *
 * const example2 = parseAbiParams(
 *   ["address", "bool"],
 *   ["0x.....", "true"]
 * ); // result: ["0x......", true]
 * ```
 * @utils
 */
export declare function parseAbiParams(constructorParamTypes: string[], constructorParamValues: unknown[]): Array<string | bigint | boolean>;
//# sourceMappingURL=parse-abi-params.d.ts.map