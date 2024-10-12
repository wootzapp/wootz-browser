export declare class SCWKeyManager {
    private readonly storage;
    private ownPrivateKey;
    private ownPublicKey;
    private peerPublicKey;
    private sharedSecret;
    getOwnPublicKey(): Promise<CryptoKey>;
    getSharedSecret(): Promise<CryptoKey | null>;
    setPeerPublicKey(key: CryptoKey): Promise<void>;
    clear(): Promise<void>;
    private generateKeyPair;
    private loadKeysIfNeeded;
    private loadKey;
    private storeKey;
}
