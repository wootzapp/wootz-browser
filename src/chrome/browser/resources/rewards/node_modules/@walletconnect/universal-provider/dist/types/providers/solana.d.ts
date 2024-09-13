/// <reference types="node" />
import Client from "@walletconnect/sign-client";
import { SessionTypes } from "@walletconnect/types";
import EventEmitter from "events";
import { IProvider, RequestParams, RpcProvidersMap, SessionNamespace, SubProviderOpts } from "../types";
declare class SolanaProvider implements IProvider {
    name: string;
    client: Client;
    httpProviders: RpcProvidersMap;
    events: EventEmitter;
    namespace: SessionNamespace;
    chainId: string;
    constructor(opts: SubProviderOpts);
    updateNamespace(namespace: SessionTypes.Namespace): void;
    requestAccounts(): string[];
    request<T = unknown>(args: RequestParams): Promise<T>;
    setDefaultChain(chainId: string, rpcUrl?: string | undefined): void;
    getDefaultChain(): string;
    private getAccounts;
    private createHttpProviders;
    private getHttpProvider;
    private setHttpProvider;
    private createHttpProvider;
}
export default SolanaProvider;
//# sourceMappingURL=solana.d.ts.map