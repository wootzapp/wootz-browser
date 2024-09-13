/// <reference types="node" />
import Client from "@walletconnect/sign-client";
import { SessionTypes } from "@walletconnect/types";
import EventEmitter from "events";
import { IProvider, RequestParams, RpcProvidersMap, SessionNamespace, SubProviderOpts } from "../types";
declare class CosmosProvider implements IProvider {
    name: string;
    client: Client;
    httpProviders: RpcProvidersMap;
    events: EventEmitter;
    namespace: SessionNamespace;
    chainId: string;
    constructor(opts: SubProviderOpts);
    updateNamespace(namespace: SessionTypes.Namespace): void;
    requestAccounts(): string[];
    getDefaultChain(): string;
    request<T = unknown>(args: RequestParams): Promise<T>;
    setDefaultChain(chainId: string, rpcUrl?: string | undefined): void;
    private getAccounts;
    private createHttpProviders;
    private getHttpProvider;
    private setHttpProvider;
    private createHttpProvider;
}
export default CosmosProvider;
//# sourceMappingURL=cosmos.d.ts.map