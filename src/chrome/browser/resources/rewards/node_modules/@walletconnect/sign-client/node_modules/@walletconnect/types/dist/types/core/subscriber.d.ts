import { IEvents } from "@walletconnect/events";
import { ErrorResponse } from "@walletconnect/jsonrpc-types";
import { Logger } from "@walletconnect/logger";
import { IRelayer, RelayerTypes } from "./relayer";
export declare namespace SubscriberTypes {
    interface Params extends RelayerTypes.SubscribeOptions {
        topic: string;
    }
    interface Active extends Params {
        id: string;
    }
}
export declare namespace SubscriberEvents {
    type Created = SubscriberTypes.Active;
    interface Deleted extends SubscriberTypes.Active {
        reason: ErrorResponse;
    }
    type Expired = Deleted;
}
export declare abstract class ISubscriberTopicMap {
    map: Map<string, string[]>;
    abstract readonly topics: string[];
    abstract set(topic: string, id: string): void;
    abstract get(topic: string): string[];
    abstract exists(topic: string, id: string): boolean;
    abstract delete(topic: string, id?: string): void;
    abstract clear(): void;
}
export declare abstract class ISubscriber extends IEvents {
    relayer: IRelayer;
    logger: Logger;
    abstract subscriptions: Map<string, SubscriberTypes.Active>;
    abstract topicMap: ISubscriberTopicMap;
    abstract pending: Map<string, SubscriberTypes.Params>;
    abstract readonly length: number;
    abstract readonly ids: string[];
    abstract readonly values: SubscriberTypes.Active[];
    abstract readonly topics: string[];
    abstract name: string;
    abstract readonly context: string;
    constructor(relayer: IRelayer, logger: Logger);
    abstract init(): Promise<void>;
    abstract subscribe(topic: string, opts?: RelayerTypes.SubscribeOptions): Promise<string | null>;
    abstract unsubscribe(topic: string, opts?: RelayerTypes.UnsubscribeOptions): Promise<void>;
    abstract isSubscribed(topic: string): Promise<boolean>;
    abstract start(): Promise<void>;
    abstract stop(): Promise<void>;
}
//# sourceMappingURL=subscriber.d.ts.map