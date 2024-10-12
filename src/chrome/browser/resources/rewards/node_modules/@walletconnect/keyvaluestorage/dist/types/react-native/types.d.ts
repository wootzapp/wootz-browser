export declare namespace AsyncStorageTypes {
    type Entry<K = string, V = any> = [K, V | null];
    type Entries<K = string, V = any> = Array<Entry<K, V>>;
    type ErrBack<V = any> = (err: Error | null, val?: V | null) => {};
    type ArrErrBack<V = any> = (err: Array<Error> | null, val?: V) => {};
}
export declare abstract class IAsyncStorage<K = string, V = any> {
    abstract store: Map<K, V | null>;
    abstract size(): number;
    abstract getStore(): Map<K, V | null>;
    abstract getItem(k: K, cb?: AsyncStorageTypes.ErrBack<V>): Promise<V | null>;
    abstract setItem(k: K, v: V, cb?: AsyncStorageTypes.ErrBack<V>): Promise<void>;
    abstract removeItem(k: K, cb?: AsyncStorageTypes.ErrBack<V>): Promise<void>;
    abstract clear(cb?: AsyncStorageTypes.ErrBack<V>): Promise<void>;
    abstract getAllKeys(cb?: AsyncStorageTypes.ErrBack<Array<K>>): Promise<Array<K>>;
    abstract multiGet(keys: Array<K>, cb?: AsyncStorageTypes.ErrBack<AsyncStorageTypes.Entries<K, V>>): Promise<AsyncStorageTypes.Entries<K, V>>;
    abstract multiSet(entries: AsyncStorageTypes.Entries<K, V>, cb?: AsyncStorageTypes.ErrBack<V>): Promise<void>;
    abstract multiRemove(keys: Array<K>, cb?: AsyncStorageTypes.ErrBack<V>): Promise<void>;
    abstract mergeItem(key: string, value: string, cb?: AsyncStorageTypes.ErrBack<string>): Promise<void>;
    abstract multiMerge(entries: AsyncStorageTypes.Entries<string, string>, cb?: AsyncStorageTypes.ArrErrBack<string>): Promise<void>;
    abstract flushGetRequests(): void;
}
//# sourceMappingURL=types.d.ts.map