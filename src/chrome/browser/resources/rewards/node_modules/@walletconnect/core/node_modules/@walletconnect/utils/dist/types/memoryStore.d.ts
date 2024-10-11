export declare abstract class MemoryStore {
    static get<T = unknown>(key: string): T | undefined;
    static set(key: string, value: unknown): void;
    static delete(key: string): void;
}
//# sourceMappingURL=memoryStore.d.ts.map