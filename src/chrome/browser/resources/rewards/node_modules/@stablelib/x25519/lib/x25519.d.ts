/**
 * Package x25519 implements X25519 key agreement.
 */
import { RandomSource } from "@stablelib/random";
export declare const PUBLIC_KEY_LENGTH = 32;
export declare const SECRET_KEY_LENGTH = 32;
export declare const SHARED_KEY_LENGTH = 32;
export declare function scalarMult(n: Uint8Array, p: Uint8Array): Uint8Array;
export declare function scalarMultBase(n: Uint8Array): Uint8Array;
export interface KeyPair {
    publicKey: Uint8Array;
    secretKey: Uint8Array;
}
export declare function generateKeyPairFromSeed(seed: Uint8Array): KeyPair;
export declare function generateKeyPair(prng?: RandomSource): KeyPair;
/**
 * Returns a shared key between our secret key and a peer's public key.
 *
 * Throws an error if the given keys are of wrong length.
 *
 * If rejectZero is true throws if the calculated shared key is all-zero.
 * From RFC 7748:
 *
 * > Protocol designers using Diffie-Hellman over the curves defined in
 * > this document must not assume "contributory behavior".  Specially,
 * > contributory behavior means that both parties' private keys
 * > contribute to the resulting shared key.  Since curve25519 and
 * > curve448 have cofactors of 8 and 4 (respectively), an input point of
 * > small order will eliminate any contribution from the other party's
 * > private key.  This situation can be detected by checking for the all-
 * > zero output, which implementations MAY do, as specified in Section 6.
 * > However, a large number of existing implementations do not do this.
 *
 * IMPORTANT: the returned key is a raw result of scalar multiplication.
 * To use it as a key material, hash it with a cryptographic hash function.
 */
export declare function sharedKey(mySecretKey: Uint8Array, theirPublicKey: Uint8Array, rejectZero?: boolean): Uint8Array;
