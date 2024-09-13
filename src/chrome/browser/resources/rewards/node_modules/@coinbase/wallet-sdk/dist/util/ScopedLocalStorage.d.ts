export declare class ScopedLocalStorage {
    private scope;
    private module?;
    constructor(scope: 'CBWSDK' | 'walletlink', module?: string | undefined);
    setItem(key: string, value: string): void;
    getItem(key: string): string | null;
    removeItem(key: string): void;
    clear(): void;
    scopedKey(key: string): string;
    static clearAll(): void;
}
