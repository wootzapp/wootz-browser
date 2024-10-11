import { Store } from "@walletconnect/core";
import { Logger } from "@walletconnect/logger";
import { ICore, SessionTypes } from "@walletconnect/types";
export declare class Session extends Store<string, SessionTypes.Struct> {
    core: ICore;
    logger: Logger;
    constructor(core: ICore, logger: Logger);
}
//# sourceMappingURL=session.d.ts.map