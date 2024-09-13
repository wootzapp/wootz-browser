import { KeyAgreement } from "@stablelib/keyagreement";
import { RandomSource } from "@stablelib/random";
/** Constants for key agreement */
export declare const OFFER_MESSAGE_LENGTH = 32;
export declare const ACCEPT_MESSAGE_LENGTH = 32;
export declare const SAVED_STATE_LENGTH = 32;
export declare const SECRET_SEED_LENGTH = 32;
/**
 * X25519 key agreement using ephemeral key pairs.
 *
 * Note that unless this key agreement is combined with an authentication
 * method, such as public key signatures, it's vulnerable to man-in-the-middle
 * attacks.
 */
export declare class X25519KeyAgreement implements KeyAgreement {
    readonly offerMessageLength = 32;
    readonly acceptMessageLength = 32;
    readonly sharedKeyLength = 32;
    readonly savedStateLength = 32;
    private _secretKey;
    private _sharedKey;
    private _offered;
    constructor(secretSeed?: Uint8Array, prng?: RandomSource);
    saveState(): Uint8Array;
    restoreState(savedState: Uint8Array): this;
    clean(): void;
    offer(): Uint8Array;
    accept(offerMsg: Uint8Array): Uint8Array;
    finish(acceptMsg: Uint8Array): this;
    getSharedKey(): Uint8Array;
}
