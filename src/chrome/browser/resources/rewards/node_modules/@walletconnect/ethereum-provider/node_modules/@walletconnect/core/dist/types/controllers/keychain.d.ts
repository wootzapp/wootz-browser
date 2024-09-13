import { Logger } from "@walletconnect/logger";
import { ICore, IKeyChain } from "@walletconnect/types";
export declare class KeyChain implements IKeyChain {
    core: ICore;
    logger: Logger;
    keychain: Map<string, string>;
    name: string;
    version: string;
    private initialized;
    private storagePrefix;
    constructor(core: ICore, logger: Logger);
    init: IKeyChain["init"];
    get context(): string;
    get storageKey(): string;
    has: IKeyChain["has"];
    set: IKeyChain["set"];
    get: IKeyChain["get"];
    del: IKeyChain["del"];
    private setKeyChain;
    private getKeyChain;
    private persist;
    private isInitialized;
}
//# sourceMappingURL=keychain.d.ts.map