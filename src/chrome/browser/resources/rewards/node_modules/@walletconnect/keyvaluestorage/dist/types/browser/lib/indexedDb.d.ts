import { IKeyValueStorage } from "../../shared";
export declare class IndexedDb implements IKeyValueStorage {
    private indexedDb;
    constructor();
    getKeys(): Promise<string[]>;
    getEntries<T = any>(): Promise<[string, T][]>;
    getItem<T = any>(key: string): Promise<T | undefined>;
    setItem<T = any>(key: string, value: T): Promise<void>;
    removeItem(key: string): Promise<void>;
}
//# sourceMappingURL=indexedDb.d.ts.map