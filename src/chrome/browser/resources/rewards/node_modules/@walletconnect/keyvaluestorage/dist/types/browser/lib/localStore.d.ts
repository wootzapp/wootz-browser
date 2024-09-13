import { IKeyValueStorage } from "../../shared";
export declare class LocalStore implements IKeyValueStorage {
    private readonly localStorage;
    getKeys(): Promise<string[]>;
    getEntries<T = any>(): Promise<[string, T][]>;
    getItem<T = any>(key: string): Promise<T | undefined>;
    setItem<T = any>(key: string, value: T): Promise<void>;
    removeItem(key: string): Promise<void>;
}
//# sourceMappingURL=localStore.d.ts.map