import { Logger } from "@walletconnect/logger";
import { ICore } from "./core";
export declare abstract class IKeyChain {
    core: ICore;
    logger: Logger;
    abstract keychain: Map<string, string>;
    abstract name: string;
    abstract readonly context: string;
    constructor(core: ICore, logger: Logger);
    abstract init(): Promise<void>;
    abstract has(tag: string, opts?: any): boolean;
    abstract set(tag: string, key: string, opts?: any): Promise<void>;
    abstract get(tag: string, opts?: any): string;
    abstract del(tag: string, opts?: any): Promise<void>;
}
//# sourceMappingURL=keychain.d.ts.map