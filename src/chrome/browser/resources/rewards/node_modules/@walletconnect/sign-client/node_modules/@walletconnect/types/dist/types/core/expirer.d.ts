import { IEvents } from "@walletconnect/events";
import { Logger } from "@walletconnect/logger";
import { ICore } from "./core";
export declare namespace ExpirerTypes {
    interface Expiration {
        target: string;
        expiry: number;
    }
    interface Created {
        target: string;
        expiration: Expiration;
    }
    interface Deleted {
        target: string;
        expiration: Expiration;
    }
    interface Expired {
        target: string;
        expiration: Expiration;
    }
}
export declare abstract class IExpirer extends IEvents {
    core: ICore;
    logger: Logger;
    abstract name: string;
    abstract readonly context: string;
    abstract readonly length: number;
    abstract readonly keys: string[];
    abstract readonly values: ExpirerTypes.Expiration[];
    constructor(core: ICore, logger: Logger);
    abstract init(): Promise<void>;
    abstract has(key: string | number): boolean;
    abstract set(key: string | number, expiry: number): void;
    abstract get(key: string | number): ExpirerTypes.Expiration;
    abstract del(key: string | number): void;
}
//# sourceMappingURL=expirer.d.ts.map