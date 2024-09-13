import { KeyAgreement } from "./keyagreement";
/**
 * ConcatKeyAgreement concatenates many key agreements into one.
 * Each message is a concatenation of underlying key agreement
 * messages, and shared key is a concatenation of all shared keys.
 */
export declare class ConcatKeyAgreement implements KeyAgreement {
    agreements: KeyAgreement[];
    readonly offerMessageLength = 0;
    readonly acceptMessageLength = 0;
    readonly sharedKeyLength = 0;
    readonly savedStateLength = 0;
    constructor(agreements: KeyAgreement[]);
    saveState(): Uint8Array;
    restoreState(savedState: Uint8Array): this;
    clean(): void;
    offer(): Uint8Array;
    accept(offerMsg: Uint8Array): Uint8Array;
    finish(acceptMsg: Uint8Array): this;
    getSharedKey(): Uint8Array;
}
