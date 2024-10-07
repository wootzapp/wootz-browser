export declare namespace RelayJsonRpc {
    interface Methods {
        publish: string;
        batchPublish: string;
        subscribe: string;
        batchSubscribe: string;
        subscription: string;
        unsubscribe: string;
        batchUnsubscribe: string;
        batchFetchMessages: string;
    }
    interface SubscribeParams {
        topic: string;
    }
    interface BatchSubscribeParams {
        topics: string[];
    }
    interface BatchFetchMessagesParams {
        topics: string[];
    }
    interface BatchUnsubscribeParams {
        subscriptions: UnsubscribeParams[];
    }
    interface PublishParams {
        topic: string;
        message: string;
        ttl: number;
        prompt?: boolean;
        tag?: number;
        attestation?: string;
    }
    interface BatchPublishParams {
        messages: PublishParams[];
    }
    interface SubscriptionData {
        topic: string;
        message: string;
        publishedAt: number;
        attestation?: string;
    }
    interface SubscriptionParams {
        id: string;
        data: SubscriptionData;
    }
    interface UnsubscribeParams {
        id: string;
        topic: string;
    }
}
//# sourceMappingURL=types.d.ts.map