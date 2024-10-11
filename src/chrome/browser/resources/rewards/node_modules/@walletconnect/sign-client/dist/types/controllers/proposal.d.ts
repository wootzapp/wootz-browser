import { Store } from "@walletconnect/core";
import { Logger } from "@walletconnect/logger";
import { ICore, ProposalTypes } from "@walletconnect/types";
export declare class Proposal extends Store<number, ProposalTypes.Struct> {
    core: ICore;
    logger: Logger;
    constructor(core: ICore, logger: Logger);
}
//# sourceMappingURL=proposal.d.ts.map