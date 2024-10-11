import { IKeyValueStorage } from "../shared";
export declare class KeyValueStorage implements IKeyValueStorage {
    private storage;
    private initialized;
    constructor();
    private setInitialized;
    getKeys(): Promise<string[]>;
    getEntries<T = any>(): Promise<[string, T][]>;
    getItem<T = any>(key: string): Promise<T | undefined>;
    setItem<T = any>(key: string, value: T): Promise<void>;
    removeItem(key: string): Promise<void>;
    private initialize;
}
export default KeyValueStorage;
//# sourceMappingURL=index.d.ts.map