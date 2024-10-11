import { Store } from "@walletconnect/core";
import { Logger } from "@walletconnect/logger";
import { ICore } from "@walletconnect/types";
export declare class AuthKey extends Store<string, {
    responseTopic: string;
    publicKey: string;
}> {
    core: ICore;
    logger: Logger;
    constructor(core: ICore, logger: Logger);
}
//# sourceMappingURL=authKey.d.ts.map