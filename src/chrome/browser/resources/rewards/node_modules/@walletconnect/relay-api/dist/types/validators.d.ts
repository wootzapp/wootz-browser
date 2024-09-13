import { JsonRpcRequest } from "@walletconnect/jsonrpc-types";
import { RelayJsonRpc } from "./types";
export declare function isSubscribeRequest(request: JsonRpcRequest): request is JsonRpcRequest<RelayJsonRpc.SubscribeParams>;
export declare function isSubscribeMethod(method: string): boolean;
export declare function isSubscribeParams(params: any): params is RelayJsonRpc.SubscribeParams;
export declare function isPublishRequest(request: JsonRpcRequest): request is JsonRpcRequest<RelayJsonRpc.PublishParams>;
export declare function isPublishMethod(method: string): boolean;
export declare function isPublishParams(params: any): params is RelayJsonRpc.PublishParams;
export declare function isUnsubscribeRequest(request: JsonRpcRequest): request is JsonRpcRequest<RelayJsonRpc.UnsubscribeParams>;
export declare function isUnsubscribeMethod(method: string): boolean;
export declare function isUnsubscribeParams(params: any): params is RelayJsonRpc.UnsubscribeParams;
export declare function isSubscriptionRequest(request: JsonRpcRequest): request is JsonRpcRequest<RelayJsonRpc.SubscriptionParams>;
export declare function isSubscriptionMethod(method: string): boolean;
export declare function isSubscriptionParams(params: any): params is RelayJsonRpc.SubscriptionParams;
//# sourceMappingURL=validators.d.ts.map