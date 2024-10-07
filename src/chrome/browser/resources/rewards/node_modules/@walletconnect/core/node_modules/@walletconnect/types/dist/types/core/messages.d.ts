import { Logger } from "@walletconnect/logger";
import { ICore } from "./core";
export declare type MessageRecord = Record<string, string>;
export declare abstract class IMessageTracker {
    logger: Logger;
    core: ICore;
    abstract messages: Map<string, MessageRecord>;
    abstract name: string;
    abstract readonly context: string;
    constructor(logger: Logger, core: ICore);
    abstract init(): Promise<void>;
    abstract set(topic: string, message: string): Promise<string>;
    abstract get(topic: string): MessageRecord;
    abstract has(topic: string, message: string): boolean;
    abstract del(topic: string): Promise<void>;
}
//# sourceMappingURL=messages.d.ts.map