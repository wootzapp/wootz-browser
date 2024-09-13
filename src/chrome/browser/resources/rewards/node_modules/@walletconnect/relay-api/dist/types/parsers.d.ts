import { JsonRpcRequest } from "@walletconnect/jsonrpc-types";
import { RelayJsonRpc } from "./types";
export declare function parseSubscribeRequest(request: JsonRpcRequest): RelayJsonRpc.SubscribeParams;
export declare function parsePublishRequest(request: JsonRpcRequest): RelayJsonRpc.PublishParams;
export declare function parseUnsubscribeRequest(request: JsonRpcRequest): RelayJsonRpc.UnsubscribeParams;
export declare function parseSubscriptionRequest(request: JsonRpcRequest): RelayJsonRpc.SubscriptionParams;
//# sourceMappingURL=parsers.d.ts.map