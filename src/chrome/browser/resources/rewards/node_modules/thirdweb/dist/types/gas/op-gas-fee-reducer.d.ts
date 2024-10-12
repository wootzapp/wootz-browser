/**
 * Via: https://twitter.com/0xjustadev/status/1758973668011434062
 *
 * Increases the gas fee value to the nearest power of 2.
 * If the value is already a power of 2 or 0, it returns the value as is.
 * Otherwise, it finds the highest power of 2 that is bigger than the given value.
 * @param value - The gas fee value to be "rounded up".
 * @returns The *increased* gas value which will result in a lower L1 gas fee, overall reducing the gas fee.
 * @internal
 */
export declare function roundUpGas(value: bigint): bigint;
//# sourceMappingURL=op-gas-fee-reducer.d.ts.map