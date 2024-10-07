export declare const DIGEST_LENGTH = 16;
/**
 * Poly1305 computes 16-byte authenticator of message using
 * a one-time 32-byte key.
 *
 * Important: key should be used for only one message,
 * it should never repeat.
 */
export declare class Poly1305 {
    readonly digestLength = 16;
    private _buffer;
    private _r;
    private _h;
    private _pad;
    private _leftover;
    private _fin;
    private _finished;
    constructor(key: Uint8Array);
    private _blocks;
    finish(mac: Uint8Array, macpos?: number): this;
    update(m: Uint8Array): this;
    digest(): Uint8Array;
    clean(): this;
}
/**
 * Returns 16-byte authenticator of data using a one-time 32-byte key.
 *
 * Important: key should be used for only one message, it should never repeat.
 */
export declare function oneTimeAuth(key: Uint8Array, data: Uint8Array): Uint8Array;
/**
 * Returns true if two authenticators are 16-byte long and equal.
 * Uses contant-time comparison to avoid leaking timing information.
 */
export declare function equal(a: Uint8Array, b: Uint8Array): boolean;
