import type { ThirdwebClient } from "../client/client.js";
export type RpcRequest = {
    jsonrpc?: "2.0";
    method: string;
    params?: unknown;
    id?: number;
};
type FetchRpcOptions = {
    requests: RpcRequest[];
    requestTimeoutMs?: number;
};
type SuccessResult<T> = {
    method?: never;
    result: T;
    error?: never;
};
type ErrorResult<T> = {
    method?: never;
    result?: never;
    error: T;
};
type Subscription<TResult, TError> = {
    method: "eth_subscription";
    error?: never;
    result?: never;
    params: {
        subscription: string;
    } & ({
        result: TResult;
        error?: never;
    } | {
        result?: never;
        error: TError;
    });
};
type RpcResponse<TResult = unknown, TError = unknown> = {
    jsonrpc: `${number}`;
    id: number;
} & (SuccessResult<TResult> | ErrorResult<TError> | Subscription<TResult, TError>);
/**
 * @internal
 */
export declare function fetchRpc(rpcUrl: string, client: ThirdwebClient, options: FetchRpcOptions): Promise<RpcResponse[]>;
type FetchSingleRpcOptions = {
    request: RpcRequest;
    requestTimeoutMs?: number;
};
/**
 * @internal
 */
export declare function fetchSingleRpc(rpcUrl: string, client: ThirdwebClient, options: FetchSingleRpcOptions): Promise<RpcResponse>;
export {};
//# sourceMappingURL=fetch-rpc.d.ts.map