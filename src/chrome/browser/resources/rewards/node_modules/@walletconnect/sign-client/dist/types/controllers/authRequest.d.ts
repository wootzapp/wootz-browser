import { Store } from "@walletconnect/core";
import { Logger } from "@walletconnect/logger";
import { AuthTypes, ICore } from "@walletconnect/types";
export declare class AuthRequest extends Store<number, AuthTypes.PendingRequest> {
    core: ICore;
    logger: Logger;
    constructor(core: ICore, logger: Logger);
}
//# sourceMappingURL=authRequest.d.ts.map