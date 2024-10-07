import { IEvents } from "@walletconnect/events";
import { Logger } from "@walletconnect/logger";
import { IRelayer, RelayerTypes } from "./relayer";
export declare namespace PublisherTypes {
    interface Params {
        topic: string;
        message: string;
        opts: Omit<RelayerTypes.PublishOptions, "internal">;
    }
}
export declare abstract class IPublisher extends IEvents {
    relayer: IRelayer;
    logger: Logger;
    abstract name: string;
    abstract readonly context: string;
    constructor(relayer: IRelayer, logger: Logger);
    abstract publish(topic: string, message: string, opts?: RelayerTypes.PublishOptions): Promise<void>;
}
//# sourceMappingURL=publisher.d.ts.map