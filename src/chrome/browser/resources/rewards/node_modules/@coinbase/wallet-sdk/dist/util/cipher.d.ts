import { EncryptedData, RPCRequest, RPCResponse } from '../core/message';
export declare function generateKeyPair(): Promise<CryptoKeyPair>;
export declare function deriveSharedSecret(ownPrivateKey: CryptoKey, peerPublicKey: CryptoKey): Promise<CryptoKey>;
export declare function encrypt(sharedSecret: CryptoKey, plainText: string): Promise<EncryptedData>;
export declare function decrypt(sharedSecret: CryptoKey, { iv, cipherText }: EncryptedData): Promise<string>;
export declare function exportKeyToHexString(type: 'public' | 'private', key: CryptoKey): Promise<string>;
export declare function importKeyFromHexString(type: 'public' | 'private', hexString: string): Promise<CryptoKey>;
export declare function encryptContent<T>(content: RPCRequest | RPCResponse<T>, sharedSecret: CryptoKey): Promise<EncryptedData>;
export declare function decryptContent<R extends RPCRequest | RPCResponse<U>, U>(encryptedData: EncryptedData, sharedSecret: CryptoKey): Promise<R>;
