import { Logger } from "@walletconnect/logger";
import { ICore, ICrypto, IKeyChain } from "@walletconnect/types";
export declare class Crypto implements ICrypto {
    core: ICore;
    logger: Logger;
    name: string;
    keychain: ICrypto["keychain"];
    readonly randomSessionIdentifier: string;
    private initialized;
    constructor(core: ICore, logger: Logger, keychain?: IKeyChain);
    init: ICrypto["init"];
    get context(): string;
    hasKeys: ICrypto["hasKeys"];
    getClientId: ICrypto["getClientId"];
    generateKeyPair: ICrypto["generateKeyPair"];
    signJWT: ICrypto["signJWT"];
    generateSharedKey: ICrypto["generateSharedKey"];
    setSymKey: ICrypto["setSymKey"];
    deleteKeyPair: ICrypto["deleteKeyPair"];
    deleteSymKey: ICrypto["deleteSymKey"];
    encode: ICrypto["encode"];
    decode: ICrypto["decode"];
    getPayloadType: ICrypto["getPayloadType"];
    getPayloadSenderPublicKey: ICrypto["getPayloadSenderPublicKey"];
    private setPrivateKey;
    private getPrivateKey;
    private getClientSeed;
    private getSymKey;
    private isInitialized;
}
//# sourceMappingURL=crypto.d.ts.map