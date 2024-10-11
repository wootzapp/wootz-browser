/**
 * Package chacha20poly1305 implements ChaCha20-Poly1305 AEAD.
 */
import { AEAD } from "@stablelib/aead";
export declare const KEY_LENGTH = 32;
export declare const NONCE_LENGTH = 12;
export declare const TAG_LENGTH = 16;
/**
 * ChaCha20-Poly1305 Authenticated Encryption with Associated Data.
 *
 * Defined in RFC7539.
 */
export declare class ChaCha20Poly1305 implements AEAD {
    readonly nonceLength = 12;
    readonly tagLength = 16;
    private _key;
    /**
     * Creates a new instance with the given 32-byte key.
     */
    constructor(key: Uint8Array);
    /**
     * Encrypts and authenticates plaintext, authenticates associated data,
     * and returns sealed ciphertext, which includes authentication tag.
     *
     * RFC7539 specifies 12 bytes for nonce. It may be this 12-byte nonce
     * ("IV"), or full 16-byte counter (called "32-bit fixed-common part")
     * and nonce.
     *
     * If dst is given (it must be the size of plaintext + the size of tag
     * length) the result will be put into it. Dst and plaintext must not
     * overlap.
     */
    seal(nonce: Uint8Array, plaintext: Uint8Array, associatedData?: Uint8Array, dst?: Uint8Array): Uint8Array;
    /**
     * Authenticates sealed ciphertext (which includes authentication tag) and
     * associated data, decrypts ciphertext and returns decrypted plaintext.
     *
     * RFC7539 specifies 12 bytes for nonce. It may be this 12-byte nonce
     * ("IV"), or full 16-byte counter (called "32-bit fixed-common part")
     * and nonce.
     *
     * If authentication fails, it returns null.
     *
     * If dst is given (it must be of ciphertext length minus tag length),
     * the result will be put into it. Dst and plaintext must not overlap.
     */
    open(nonce: Uint8Array, sealed: Uint8Array, associatedData?: Uint8Array, dst?: Uint8Array): Uint8Array | null;
    clean(): this;
    private _authenticate;
}
