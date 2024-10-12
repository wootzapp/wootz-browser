/**
 * Package random provides functions to access system's
 * cryptographically secure random byte generator.
 */
import { RandomSource } from "./source";
import { SystemRandomSource } from "./source/system";
export { RandomSource } from "./source";
export declare const defaultRandomSource: SystemRandomSource;
export declare function randomBytes(length: number, prng?: RandomSource): Uint8Array;
/**
 * Returns a uniformly random unsigned 32-bit integer.
 */
export declare function randomUint32(prng?: RandomSource): number;
/**
 * Returns a uniform random string of the given length
 * with characters from the given charset.
 *
 * Charset must not have more than 256 characters.
 *
 * Default charset generates case-sensitive alphanumeric
 * strings (0-9, A-Z, a-z).
 */
export declare function randomString(length: number, charset?: string, prng?: RandomSource): string;
/**
 * Returns uniform random string containing at least the given
 * number of bits of entropy.
 *
 * For example, randomStringForEntropy(128) will return a 22-character
 * alphanumeric string, while randomStringForEntropy(128, "0123456789")
 * will return a 39-character numeric string, both will contain at
 * least 128 bits of entropy.
 *
 * Default charset generates case-sensitive alphanumeric
 * strings (0-9, A-Z, a-z).
 */
export declare function randomStringForEntropy(bits: number, charset?: string, prng?: RandomSource): string;
