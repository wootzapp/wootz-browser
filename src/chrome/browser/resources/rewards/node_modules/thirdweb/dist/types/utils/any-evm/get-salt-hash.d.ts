import type { Hex } from "../encoding/hex.js";
/**
 * Calculates the salt hash for a given bytecode.
 * @param bytecode - The bytecode to calculate the salt hash for.
 * @returns The salt hash of the bytecode.
 * @example
 * ```ts
 * import { getSaltHash } from "thirdweb";
 * const saltHash = getSaltHash(bytecode);
 * ```
 * @utils
 */
export declare function getSaltHash(bytecode: string): Hex;
//# sourceMappingURL=get-salt-hash.d.ts.map