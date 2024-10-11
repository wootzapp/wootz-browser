/**
 * Converts a given number of units to a string representation with a specified number of decimal places.
 * @param units - The number of units to convert.
 * @param decimals - The number of decimal places to include in the string representation.
 * @returns The string representation of the converted units.
 * @example
 * ```ts
 * import { toTokens } from "thirdweb/utils";
 * toTokens(1000000000000000000n, 18)
 * // '1'
 * ```
 * @utils
 */
export declare function toTokens(units: bigint, decimals: number): string;
/**
 * Converts a value from wei to ether.
 * @param wei The value in wei to be converted.
 * @returns The converted value in ether.
 * @example
 * ```ts
 * import { toEther } from "thirdweb/utils";
 * toEther(1000000000000000000n)
 * // '1'
 * ```
 * @utils
 */
export declare function toEther(wei: bigint): string;
/**
 * Converts a string representation of a number with decimal places to a BigInt representation.
 * @param tokens - The string representation of the number, including the integer and fraction parts.
 * @param decimals - The number of decimal places to include in the BigInt representation.
 * @returns The BigInt representation of the number.
 * @example
 * ```ts
 * import { toUnits } from "thirdweb/utils";
 * toUnits('1', 18)
 * // 1000000000000000000n
 * ```
 * @utils
 */
export declare function toUnits(tokens: string, decimals: number): bigint;
/**
 * Converts the specified number of tokens to Wei.
 * @param tokens The number of tokens to convert.
 * @returns The converted value in Wei.
 * @example
 * ```ts
 * import { toWei } from "thirdweb/utils";
 * toWei('1')
 * // 1000000000000000000n
 * ```
 * @utils
 */
export declare function toWei(tokens: string): bigint;
/**
 * Converts the specified number from gwei to wei.
 * @param gwei The number of gwei to convert.
 * @returns The converted value in wei.
 * @example
 * ```ts
 * import { fromGwei } from "thirdweb/utils";
 * fromGwei('1')
 * // 1000000000n
 * ```
 * @utils
 */
export declare function fromGwei(gwei: string): bigint;
//# sourceMappingURL=units.d.ts.map