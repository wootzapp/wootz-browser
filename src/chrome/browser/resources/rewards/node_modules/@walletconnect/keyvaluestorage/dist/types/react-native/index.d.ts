import { IKeyValueStorage } from "../shared";
export declare class KeyValueStorage implements IKeyValueStorage {
    private readonly asyncStorage;
    getKeys(): Promise<string[]>;
    getEntries<T = any>(): Promise<[string, T][]>;
    getItem<T = any>(key: string): Promise<T | undefined>;
    setItem<T = any>(key: string, value: T): Promise<void>;
    removeItem(key: string): Promise<void>;
}
export default KeyValueStorage;
//# sourceMappingURL=index.d.ts.map