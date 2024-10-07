import { Store } from "@walletconnect/core";
import { Logger } from "@walletconnect/logger";
import { ICore, PendingRequestTypes } from "@walletconnect/types";
export declare class PendingRequest extends Store<number, PendingRequestTypes.Struct> {
    core: ICore;
    logger: Logger;
    constructor(core: ICore, logger: Logger);
}
//# sourceMappingURL=pendingRequest.d.ts.map