/**
 * Package sha256 implements SHA-2-256 cryptographic hash function.
 */
import { SerializableHash } from "@stablelib/hash";
export declare const DIGEST_LENGTH = 32;
export declare const BLOCK_SIZE = 64;
/**
 * SHA2-256 cryptographic hash algorithm.
 */
export declare class SHA256 implements SerializableHash {
    /** Length of hash output */
    readonly digestLength: number;
    /** Block size */
    readonly blockSize: number;
    protected _state: Int32Array;
    private _temp;
    private _buffer;
    private _bufferLength;
    private _bytesHashed;
    private _finished;
    constructor();
    protected _initState(): void;
    /**
     * Resets hash state making it possible
     * to re-use this instance to hash other data.
     */
    reset(): this;
    /**
     * Cleans internal buffers and resets hash state.
     */
    clean(): void;
    /**
     * Updates hash state with the given data.
     *
     * Throws error when trying to update already finalized hash:
     * instance must be reset to update it again.
     */
    update(data: Uint8Array, dataLength?: number): this;
    /**
     * Finalizes hash state and puts hash into out.
     * If hash was already finalized, puts the same value.
     */
    finish(out: Uint8Array): this;
    /**
     * Returns the final hash digest.
     */
    digest(): Uint8Array;
    /**
     * Function useful for HMAC/PBKDF2 optimization.
     * Returns hash state to be used with restoreState().
     * Only chain value is saved, not buffers or other
     * state variables.
     */
    saveState(): SavedState;
    /**
     * Function useful for HMAC/PBKDF2 optimization.
     * Restores state saved by saveState() and sets bytesHashed
     * to the given value.
     */
    restoreState(savedState: SavedState): this;
    /**
     * Cleans state returned by saveState().
     */
    cleanSavedState(savedState: SavedState): void;
}
export declare type SavedState = {
    state: Int32Array;
    buffer: Uint8Array | undefined;
    bufferLength: number;
    bytesHashed: number;
};
export declare function hash(data: Uint8Array): Uint8Array;
