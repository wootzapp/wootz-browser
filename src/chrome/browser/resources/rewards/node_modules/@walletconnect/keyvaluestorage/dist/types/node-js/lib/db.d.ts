interface DbKeyValueStorageOptions {
    dbName: string;
}
export declare const MEMORY_DB = ":memory:";
export default class Db {
    private static instances;
    database: any;
    private writeActionsQueue;
    private constructor();
    static create(opts: DbKeyValueStorageOptions): Db;
    getKeys(): Promise<string[]>;
    getEntries<T = any>(): Promise<[string, T][]>;
    private onWriteAction;
    private startWriteActions;
    getItem<T = any>(key: string): Promise<T | undefined>;
    setItem<_T = any>(key: string, value: any): Promise<void>;
    removeItem(key: string): Promise<void>;
}
export {};
//# sourceMappingURL=db.d.ts.map