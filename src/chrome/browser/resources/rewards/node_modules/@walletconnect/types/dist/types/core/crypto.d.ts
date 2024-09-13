import { JsonRpcPayload } from "@walletconnect/jsonrpc-types";
import { Logger } from "@walletconnect/logger";
import { ICore } from "./core";
import { IKeyChain } from "./keychain";
export declare namespace CryptoTypes {
    interface Participant {
        publicKey: string;
    }
    interface KeyPair {
        privateKey: string;
        publicKey: string;
    }
    interface EncryptParams {
        message: string;
        symKey: string;
        type?: number;
        iv?: string;
        senderPublicKey?: string;
    }
    interface DecryptParams {
        symKey: string;
        encoded: string;
    }
    interface EncodingParams {
        type: Uint8Array;
        sealed: Uint8Array;
        iv: Uint8Array;
        senderPublicKey?: Uint8Array;
    }
    interface EncodeOptions {
        type?: number;
        senderPublicKey?: string;
        receiverPublicKey?: string;
    }
    interface DecodeOptions {
        receiverPublicKey?: string;
    }
    interface EncodingValidation {
        type: number;
        senderPublicKey?: string;
        receiverPublicKey?: string;
    }
    interface TypeOneParams {
        type: 1;
        senderPublicKey: string;
        receiverPublicKey: string;
    }
}
export declare abstract class ICrypto {
    core: ICore;
    logger: Logger;
    abstract name: string;
    abstract readonly context: string;
    abstract keychain: IKeyChain;
    abstract readonly randomSessionIdentifier: string;
    constructor(core: ICore, logger: Logger, keychain?: IKeyChain);
    abstract init(): Promise<void>;
    abstract hasKeys(tag: string): boolean;
    abstract getClientId(): Promise<string>;
    abstract generateKeyPair(): Promise<string>;
    abstract generateSharedKey(selfPublicKey: string, peerPublicKey: string, overrideTopic?: string): Promise<string>;
    abstract setSymKey(symKey: string, overrideTopic?: string): Promise<string>;
    abstract deleteKeyPair(publicKey: string): Promise<void>;
    abstract deleteSymKey(topic: string): Promise<void>;
    abstract encode(topic: string, payload: JsonRpcPayload, opts?: CryptoTypes.EncodeOptions): Promise<string>;
    abstract decode(topic: string, encoded: string, opts?: CryptoTypes.DecodeOptions): Promise<JsonRpcPayload>;
    abstract signJWT(aud: string): Promise<string>;
    abstract getPayloadType(encoded: string): number;
    abstract getPayloadSenderPublicKey(encoded: string): string | undefined;
}
//# sourceMappingURL=crypto.d.ts.map