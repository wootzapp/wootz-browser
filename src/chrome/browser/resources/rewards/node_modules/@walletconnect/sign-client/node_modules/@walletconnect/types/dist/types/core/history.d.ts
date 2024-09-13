import { IEvents } from "@walletconnect/events";
import { ErrorResponse, JsonRpcRequest, JsonRpcResponse, RequestArguments } from "@walletconnect/jsonrpc-types";
import { Logger } from "@walletconnect/logger";
import { ICore } from "./core";
export interface JsonRpcRecord {
    id: number;
    topic: string;
    request: RequestArguments;
    chainId?: string;
    response?: {
        result: any;
    } | {
        error: ErrorResponse;
    };
    expiry?: number;
}
export interface RequestEvent {
    topic: string;
    request: JsonRpcRequest;
    chainId?: string;
}
export declare abstract class IJsonRpcHistory extends IEvents {
    core: ICore;
    logger: Logger;
    records: Map<number, JsonRpcRecord>;
    abstract readonly context: string;
    abstract readonly size: number;
    abstract readonly keys: number[];
    abstract readonly values: JsonRpcRecord[];
    abstract readonly pending: RequestEvent[];
    constructor(core: ICore, logger: Logger);
    abstract init(): Promise<void>;
    abstract set(topic: string, request: JsonRpcRequest, chainId?: string): void;
    abstract get(topic: string, id: number): Promise<JsonRpcRecord>;
    abstract resolve(response: JsonRpcResponse): Promise<void>;
    abstract delete(topic: string, id?: number): void;
    abstract exists(topic: string, id: number): Promise<boolean>;
}
//# sourceMappingURL=history.d.ts.map