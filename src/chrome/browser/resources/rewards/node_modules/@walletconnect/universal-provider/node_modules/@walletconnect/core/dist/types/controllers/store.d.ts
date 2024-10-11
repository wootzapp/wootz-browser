import { Logger } from "@walletconnect/logger";
import { ICore, IStore } from "@walletconnect/types";
export declare class Store<Key, Data extends Record<string, any>> extends IStore<Key, Data> {
    core: ICore;
    logger: Logger;
    name: string;
    map: Map<Key, Data>;
    version: string;
    private cached;
    private initialized;
    private getKey;
    private storagePrefix;
    private recentlyDeleted;
    private recentlyDeletedLimit;
    constructor(core: ICore, logger: Logger, name: string, storagePrefix?: string, getKey?: Store<Key, Data>["getKey"]);
    init: IStore<Key, Data>["init"];
    get context(): string;
    get storageKey(): string;
    get length(): number;
    get keys(): Key[];
    get values(): Data[];
    set: IStore<Key, Data>["set"];
    get: IStore<Key, Data>["get"];
    getAll: IStore<Key, Data>["getAll"];
    update: IStore<Key, Data>["update"];
    delete: IStore<Key, Data>["delete"];
    private addToRecentlyDeleted;
    private setDataStore;
    private getDataStore;
    private getData;
    private persist;
    private restore;
    private isInitialized;
}
//# sourceMappingURL=store.d.ts.map