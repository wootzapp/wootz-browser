export declare const standardErrors: {
    rpc: {
        parse: <T>(arg?: EthErrorsArg<T> | undefined) => EthereumRpcError<T>;
        invalidRequest: <T_1>(arg?: EthErrorsArg<T_1> | undefined) => EthereumRpcError<T_1>;
        invalidParams: <T_2>(arg?: EthErrorsArg<T_2> | undefined) => EthereumRpcError<T_2>;
        methodNotFound: <T_3>(arg?: EthErrorsArg<T_3> | undefined) => EthereumRpcError<T_3>;
        internal: <T_4>(arg?: EthErrorsArg<T_4> | undefined) => EthereumRpcError<T_4>;
        server: <T_5>(opts: ServerErrorOptions<T_5>) => EthereumRpcError<T_5>;
        invalidInput: <T_6>(arg?: EthErrorsArg<T_6> | undefined) => EthereumRpcError<T_6>;
        resourceNotFound: <T_7>(arg?: EthErrorsArg<T_7> | undefined) => EthereumRpcError<T_7>;
        resourceUnavailable: <T_8>(arg?: EthErrorsArg<T_8> | undefined) => EthereumRpcError<T_8>;
        transactionRejected: <T_9>(arg?: EthErrorsArg<T_9> | undefined) => EthereumRpcError<T_9>;
        methodNotSupported: <T_10>(arg?: EthErrorsArg<T_10> | undefined) => EthereumRpcError<T_10>;
        limitExceeded: <T_11>(arg?: EthErrorsArg<T_11> | undefined) => EthereumRpcError<T_11>;
    };
    provider: {
        userRejectedRequest: <T_12>(arg?: EthErrorsArg<T_12> | undefined) => EthereumProviderError<T_12>;
        unauthorized: <T_13>(arg?: EthErrorsArg<T_13> | undefined) => EthereumProviderError<T_13>;
        unsupportedMethod: <T_14>(arg?: EthErrorsArg<T_14> | undefined) => EthereumProviderError<T_14>;
        disconnected: <T_15>(arg?: EthErrorsArg<T_15> | undefined) => EthereumProviderError<T_15>;
        chainDisconnected: <T_16>(arg?: EthErrorsArg<T_16> | undefined) => EthereumProviderError<T_16>;
        unsupportedChain: <T_17>(arg?: EthErrorsArg<T_17> | undefined) => EthereumProviderError<T_17>;
        custom: <T_18>(opts: CustomErrorArg<T_18>) => EthereumProviderError<T_18>;
    };
};
interface EthereumErrorOptions<T> {
    message?: string;
    data?: T;
}
interface ServerErrorOptions<T> extends EthereumErrorOptions<T> {
    code: number;
}
type CustomErrorArg<T> = ServerErrorOptions<T>;
type EthErrorsArg<T> = EthereumErrorOptions<T> | string;
declare class EthereumRpcError<T> extends Error {
    code: number;
    data?: T;
    constructor(code: number, message: string, data?: T);
}
declare class EthereumProviderError<T> extends EthereumRpcError<T> {
    /**
     * Create an Ethereum Provider JSON-RPC error.
     * `code` must be an integer in the 1000 <= 4999 range.
     */
    constructor(code: number, message: string, data?: T);
}
export {};
