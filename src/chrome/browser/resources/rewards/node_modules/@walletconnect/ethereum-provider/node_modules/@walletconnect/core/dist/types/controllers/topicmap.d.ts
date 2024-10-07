import { ISubscriberTopicMap } from "@walletconnect/types";
export declare class SubscriberTopicMap implements ISubscriberTopicMap {
    map: Map<string, string[]>;
    get topics(): string[];
    set: ISubscriberTopicMap["set"];
    get: ISubscriberTopicMap["get"];
    exists: ISubscriberTopicMap["exists"];
    delete: ISubscriberTopicMap["delete"];
    clear: ISubscriberTopicMap["clear"];
}
//# sourceMappingURL=topicmap.d.ts.map