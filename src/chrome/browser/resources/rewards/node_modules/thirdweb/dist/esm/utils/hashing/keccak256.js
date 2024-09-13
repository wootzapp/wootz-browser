import { keccak_256 } from "@noble/hashes/sha3";
import { hexToUint8Array, isHex, uint8ArrayToHex, } from "../encoding/hex.js";
/**
 * Calculates the Keccak-256 hash of the given value.
 * @param value - The value to hash, either as a hexadecimal string or a Uint8Array.
 * @param to - The desired output format of the hash (optional). Defaults to 'hex'.
 * @returns The Keccak-256 hash of the value in the specified format.
 * @example
 * ```ts
 * import { keccak256 } from "thirdweb/utils";
 * const hash = keccak256("0x1234");
 * ```
 * @utils
 */
export function keccak256(value, to) {
    const bytes = keccak_256(isHex(value, { strict: false }) ? hexToUint8Array(value) : value);
    if (to === "bytes") {
        return bytes;
    }
    // default fall through to hex
    return uint8ArrayToHex(bytes);
}
//# sourceMappingURL=keccak256.js.map