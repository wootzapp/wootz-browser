import { sha256 as noble_sha256 } from "@noble/hashes/sha256";
import { hexToUint8Array, isHex, uint8ArrayToHex, } from "../encoding/hex.js";
/**
 * Calculates the SHA256 hash of the given value.
 * @param value - The value to hash. It can be either a hexadecimal string or a Uint8Array.
 * @param to - (Optional) The desired output format of the hash. Defaults to 'hex'.
 * @returns The SHA256 hash of the value in the specified format.
 * @example
 * ```ts
 * import { sha256 } from "thirdweb/utils";
 * const hash = sha256("0x1234");
 * ```
 * @utils
 */
export function sha256(value, to) {
    const bytes = noble_sha256(isHex(value, { strict: false }) ? hexToUint8Array(value) : value);
    if (to === "bytes") {
        return bytes;
    }
    return uint8ArrayToHex(bytes);
}
//# sourceMappingURL=sha256.js.map