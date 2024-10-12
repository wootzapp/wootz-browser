import { Logger } from "@walletconnect/logger";
import { IKeyValueStorage } from "@walletconnect/keyvaluestorage";
import { ICore } from "./core";
export declare namespace Verify {
    interface Context {
        verified: {
            origin: string;
            validation: "UNKNOWN" | "VALID" | "INVALID";
            verifyUrl: string;
            isScam?: boolean;
        };
    }
}
export declare abstract class IVerify {
    core: ICore;
    logger: Logger;
    store: IKeyValueStorage;
    abstract readonly context: string;
    constructor(core: ICore, logger: Logger, store: IKeyValueStorage);
    abstract register(params: {
        id: string;
        decryptedId: string;
    }): Promise<string | undefined>;
    abstract resolve(params: {
        attestationId?: string;
        hash?: string;
        encryptedId?: string;
        verifyUrl?: string;
    }): Promise<{
        origin: string;
        isScam?: boolean;
    }>;
}
//# sourceMappingURL=verify.d.ts.map