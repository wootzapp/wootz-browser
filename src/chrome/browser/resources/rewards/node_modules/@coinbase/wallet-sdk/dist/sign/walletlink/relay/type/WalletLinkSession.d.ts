import { ScopedLocalStorage } from '../../../../util/ScopedLocalStorage';
export declare class WalletLinkSession {
    private readonly _id;
    private readonly _secret;
    private readonly _key;
    private readonly _storage;
    private _linked;
    constructor(storage: ScopedLocalStorage, id?: string, secret?: string, linked?: boolean);
    static load(storage: ScopedLocalStorage): WalletLinkSession | null;
    get id(): string;
    get secret(): string;
    get key(): string;
    get linked(): boolean;
    set linked(val: boolean);
    save(): WalletLinkSession;
    private persistLinked;
}
