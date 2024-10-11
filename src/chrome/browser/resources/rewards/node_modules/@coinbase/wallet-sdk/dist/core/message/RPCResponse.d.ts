import { SerializedEthereumRpcError } from '../error';
export type RPCResponse<T> = {
    result: {
        value: T;
    } | {
        error: SerializedEthereumRpcError;
    };
    data?: {
        chains?: {
            [key: number]: string;
        };
        capabilities?: Record<`0x${string}`, Record<string, unknown>>;
    };
};
