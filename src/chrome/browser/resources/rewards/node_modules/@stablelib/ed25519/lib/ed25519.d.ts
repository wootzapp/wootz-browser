/**
 * Package ed25519 implements Ed25519 public-key signature algorithm.
 */
import { RandomSource } from "@stablelib/random";
export declare const SIGNATURE_LENGTH = 64;
export declare const PUBLIC_KEY_LENGTH = 32;
export declare const SECRET_KEY_LENGTH = 64;
export declare const SEED_LENGTH = 32;
export interface KeyPair {
    publicKey: Uint8Array;
    secretKey: Uint8Array;
}
export declare function generateKeyPairFromSeed(seed: Uint8Array): KeyPair;
export declare function generateKeyPair(prng?: RandomSource): KeyPair;
export declare function extractPublicKeyFromSecretKey(secretKey: Uint8Array): Uint8Array;
export declare function sign(secretKey: Uint8Array, message: Uint8Array): Uint8Array;
export declare function verify(publicKey: Uint8Array, message: Uint8Array, signature: Uint8Array): boolean;
/**
 * Convert Ed25519 public key to X25519 public key.
 *
 * Throws if given an invalid public key.
 */
export declare function convertPublicKeyToX25519(publicKey: Uint8Array): Uint8Array;
/**
 *  Convert Ed25519 secret (private) key to X25519 secret key.
 */
export declare function convertSecretKeyToX25519(secretKey: Uint8Array): Uint8Array;
