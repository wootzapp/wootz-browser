import { Logger } from "@walletconnect/logger";
import { ICore, IVerify } from "@walletconnect/types";
import { IKeyValueStorage } from "@walletconnect/keyvaluestorage";
export declare class Verify extends IVerify {
    core: ICore;
    logger: Logger;
    store: IKeyValueStorage;
    name: string;
    private abortController;
    private isDevEnv;
    private verifyUrlV3;
    private storagePrefix;
    private version;
    private publicKey?;
    private fetchPromise?;
    constructor(core: ICore, logger: Logger, store: IKeyValueStorage);
    get storeKey(): string;
    init: () => Promise<void>;
    register: IVerify["register"];
    resolve: IVerify["resolve"];
    get context(): string;
    private fetchAttestation;
    private startAbortTimer;
    private getVerifyUrl;
    private fetchPublicKey;
    private persistPublicKey;
    private removePublicKey;
    private isValidJwtAttestation;
    private getPublicKey;
    private fetchAndPersistPublicKey;
    private validateAttestation;
}
//# sourceMappingURL=verify.d.ts.map