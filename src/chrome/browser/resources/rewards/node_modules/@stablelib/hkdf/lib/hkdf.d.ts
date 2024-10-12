/**
 * Package hkdf implements HKDF key derivation function.
 */
import { Hash } from "@stablelib/hash";
/**
 * HMAC-based Extract-and-Expand Key Derivation Function.
 *
 * Implements HKDF from RFC5869.
 *
 * Expands the given master key with salt and info into
 * a limited stream of key material.
 */
export declare class HKDF {
    private _hmac;
    private _buffer;
    private _bufpos;
    private _counter;
    private _hash;
    private _info?;
    /**
     * Create a new HKDF instance for the given hash function
     * with the master key, optional salt, and info.
     *
     * - Master key is a high-entropy secret key (not a password).
     * - Salt is a non-secret random value.
     * - Info is application- and/or context-specific information.
     */
    constructor(hash: new () => Hash, key: Uint8Array, salt?: Uint8Array, info?: Uint8Array);
    private _fillBuffer;
    /**
     * Expand returns next key material of the given length.
     *
     * It throws if expansion limit is reached (which is
     * 254 digests of the underlying HMAC function).
     */
    expand(length: number): Uint8Array;
    clean(): void;
}
