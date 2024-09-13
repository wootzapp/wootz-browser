/**
 * Package hmac implements HMAC algorithm.
 */
import { Hash, SerializableHash } from "@stablelib/hash";
import { equal as constantTimeEqual } from "@stablelib/constant-time";
/**
 *  HMAC implements hash-based message authentication algorithm.
 */
export declare class HMAC implements SerializableHash {
    readonly blockSize: number;
    readonly digestLength: number;
    private _inner;
    private _outer;
    private _finished;
    private _innerKeyedState;
    private _outerKeyedState;
    /**
     * Constructs a new HMAC with the given Hash and secret key.
     */
    constructor(hash: new () => Hash | SerializableHash, key: Uint8Array);
    /**
     * Returns HMAC state to the state initialized with key
     * to make it possible to run HMAC over the other data with the same
     * key without creating a new instance.
     */
    reset(): this;
    /**
     * Cleans HMAC state.
     */
    clean(): void;
    /**
     * Updates state with provided data.
     */
    update(data: Uint8Array): this;
    /**
     * Finalizes HMAC and puts the result in out.
     */
    finish(out: Uint8Array): this;
    /**
     * Returns the computed message authentication code.
     */
    digest(): Uint8Array;
    /**
     * Saves HMAC state.
     * This function is needed for PBKDF2 optimization.
     */
    saveState(): any;
    restoreState(savedState: any): this;
    cleanSavedState(savedState: any): void;
}
/**
 * Returns HMAC using the given hash constructor for the key over data.
 */
export declare function hmac(hash: new () => Hash, key: Uint8Array, data: Uint8Array): Uint8Array;
/**
 * Returns true if two HMAC digests are equal.
 * Uses constant-time comparison to avoid leaking timing information.
 *
 * Example:
 *
 *    const receivedDigest = ...
 *    const realDigest = hmac(SHA256, key, data);
 *    if (!equal(receivedDigest, realDigest)) {
 *        throw new Error("Authentication error");
 *    }
 */
export declare const equal: typeof constantTimeEqual;
