/// <reference types="node" />
import { Logger } from "@walletconnect/logger";
import { ExpirerTypes, ICore, IExpirer } from "@walletconnect/types";
import { EventEmitter } from "events";
export declare class Expirer extends IExpirer {
    core: ICore;
    logger: Logger;
    expirations: Map<string, ExpirerTypes.Expiration>;
    events: EventEmitter;
    name: string;
    version: string;
    private cached;
    private initialized;
    private storagePrefix;
    constructor(core: ICore, logger: Logger);
    init: IExpirer["init"];
    get context(): string;
    get storageKey(): string;
    get length(): number;
    get keys(): string[];
    get values(): ExpirerTypes.Expiration[];
    has: IExpirer["has"];
    set: IExpirer["set"];
    get: IExpirer["get"];
    del: IExpirer["del"];
    on: IExpirer["on"];
    once: IExpirer["once"];
    off: IExpirer["off"];
    removeListener: IExpirer["removeListener"];
    private formatTarget;
    private setExpirations;
    private getExpirations;
    private persist;
    private restore;
    private getExpiration;
    private checkExpiry;
    private expire;
    private checkExpirations;
    private registerEventListeners;
    private isInitialized;
}
//# sourceMappingURL=expirer.d.ts.map