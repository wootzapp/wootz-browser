import { stringToBytes } from "../encoding/to-bytes.js";
import { keccak256 } from "../hashing/keccak256.js";
/**
 * Calculates the keccak ID of the given input.
 * @param input - The input value to calculate the keccak ID for.
 * @returns The keccak ID as a Hex string.
 * @example
 * ```ts
 * import { keccackId } from "thirdweb/utils";
 * const keccakId = keccackId(input);
 * ```
 * @utils
 */
export function keccakId(input) {
    return keccak256(stringToBytes(input));
}
//# sourceMappingURL=keccak-id.js.map