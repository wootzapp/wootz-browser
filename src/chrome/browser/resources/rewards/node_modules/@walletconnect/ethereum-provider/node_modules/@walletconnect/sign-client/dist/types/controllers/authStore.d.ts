import { Logger } from "@walletconnect/logger";
import { IAuth, ICore } from "@walletconnect/types";
export declare class AuthStore {
    core: ICore;
    logger: Logger;
    authKeys: IAuth["authKeys"];
    pairingTopics: IAuth["pairingTopics"];
    requests: IAuth["requests"];
    constructor(core: ICore, logger: Logger);
    init(): Promise<void>;
}
//# sourceMappingURL=authStore.d.ts.map