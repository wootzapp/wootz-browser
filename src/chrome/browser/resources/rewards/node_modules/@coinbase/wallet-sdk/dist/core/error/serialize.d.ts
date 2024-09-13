import { SerializedEthereumRpcError } from './utils';
/**
 * Serializes an error to a format that is compatible with the Ethereum JSON RPC error format.
 * See https://docs.cloud.coinbase.com/wallet-sdk/docs/errors
 * for more information.
 */
export declare function serializeError(error: unknown, requestOrMethod?: JSONRPCRequest | JSONRPCRequest[] | string): SerializedError;
export interface SerializedError extends SerializedEthereumRpcError {
    docUrl: string;
}
interface JSONRPCRequest {
    method: string;
}
export {};
