import { Logger } from "@walletconnect/logger";
import { ICore, IMessageTracker, MessageRecord } from "@walletconnect/types";
export declare class MessageTracker extends IMessageTracker {
    logger: Logger;
    core: ICore;
    messages: Map<string, MessageRecord>;
    name: string;
    version: string;
    private initialized;
    private storagePrefix;
    constructor(logger: Logger, core: ICore);
    init: IMessageTracker["init"];
    get context(): string;
    get storageKey(): string;
    set: IMessageTracker["set"];
    get: IMessageTracker["get"];
    has: IMessageTracker["has"];
    del: IMessageTracker["del"];
    private setRelayerMessages;
    private getRelayerMessages;
    private persist;
    private isInitialized;
}
//# sourceMappingURL=messages.d.ts.map