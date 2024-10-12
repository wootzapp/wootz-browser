import { IKeyValueStorage, KeyValueStorageOptions } from "../shared";
export declare class KeyValueStorage implements IKeyValueStorage {
    private database;
    private initialized;
    constructor(opts?: KeyValueStorageOptions);
    private setInitialized;
    getKeys(): Promise<string[]>;
    getEntries<T = any>(): Promise<[string, T][]>;
    getItem<T = any>(key: string): Promise<T | undefined>;
    setItem<_T = any>(key: string, value: any): Promise<void>;
    removeItem(key: string): Promise<void>;
    private initialize;
}
export default KeyValueStorage;
//# sourceMappingURL=index.d.ts.map