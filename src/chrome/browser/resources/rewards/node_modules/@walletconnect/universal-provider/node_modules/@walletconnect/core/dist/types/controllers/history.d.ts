/// <reference types="node" />
import { Logger } from "@walletconnect/logger";
import { IJsonRpcHistory, JsonRpcRecord, RequestEvent, ICore } from "@walletconnect/types";
import { EventEmitter } from "events";
export declare class JsonRpcHistory extends IJsonRpcHistory {
    core: ICore;
    logger: Logger;
    records: Map<number, JsonRpcRecord>;
    events: EventEmitter;
    name: string;
    version: string;
    private cached;
    private initialized;
    private storagePrefix;
    constructor(core: ICore, logger: Logger);
    init: IJsonRpcHistory["init"];
    get context(): string;
    get storageKey(): string;
    get size(): number;
    get keys(): number[];
    get values(): JsonRpcRecord[];
    get pending(): RequestEvent[];
    set: IJsonRpcHistory["set"];
    resolve: IJsonRpcHistory["resolve"];
    get: IJsonRpcHistory["get"];
    delete: IJsonRpcHistory["delete"];
    exists: IJsonRpcHistory["exists"];
    on: IJsonRpcHistory["on"];
    once: IJsonRpcHistory["once"];
    off: IJsonRpcHistory["off"];
    removeListener: IJsonRpcHistory["removeListener"];
    private setJsonRpcRecords;
    private getJsonRpcRecords;
    private getRecord;
    private persist;
    private restore;
    private registerEventListeners;
    private cleanup;
    private isInitialized;
}
//# sourceMappingURL=history.d.ts.map