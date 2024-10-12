import { Store } from "@walletconnect/core";
import { Logger } from "@walletconnect/logger";
import { ICore } from "@walletconnect/types";
export declare class AuthPairingTopic extends Store<string, {
    topic: string;
    pairingTopic: string;
}> {
    core: ICore;
    logger: Logger;
    constructor(core: ICore, logger: Logger);
}
//# sourceMappingURL=authPairingTopic.d.ts.map