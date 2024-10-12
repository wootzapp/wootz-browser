/// <reference types="node" />
import { EventEmitter } from "events";
import { KeyValueStorageOptions } from "@walletconnect/keyvaluestorage";
import { IEthereumProvider as IProvider, IEthereumProviderEvents, ProviderAccounts, RequestArguments, QrModalOptions } from "./types";
import { Metadata, Namespace, UniversalProvider, UniversalProviderOpts } from "@walletconnect/universal-provider";
import { AuthTypes, SessionTypes } from "@walletconnect/types";
import { JsonRpcResult } from "@walletconnect/jsonrpc-types";
export declare type RpcMethod = "personal_sign" | "eth_sendTransaction" | "eth_accounts" | "eth_requestAccounts" | "eth_call" | "eth_getBalance" | "eth_sendRawTransaction" | "eth_sign" | "eth_signTransaction" | "eth_signTypedData" | "eth_signTypedData_v3" | "eth_signTypedData_v4" | "wallet_switchEthereumChain" | "wallet_addEthereumChain" | "wallet_getPermissions" | "wallet_requestPermissions" | "wallet_registerOnboarding" | "wallet_watchAsset" | "wallet_scanQRCode" | "wallet_sendCalls" | "wallet_getCapabilities" | "wallet_getCallsStatus" | "wallet_showCallsStatus";
export declare type RpcEvent = "accountsChanged" | "chainChanged" | "message" | "disconnect" | "connect";
export interface EthereumRpcMap {
    [chainId: string]: string;
}
export interface SessionEvent {
    event: {
        name: string;
        data: any;
    };
    chainId: string;
}
export interface EthereumRpcConfig {
    chains: string[];
    optionalChains: string[];
    methods: string[];
    optionalMethods?: string[];
    events: string[];
    optionalEvents?: string[];
    rpcMap: EthereumRpcMap;
    projectId: string;
    metadata?: Metadata;
    showQrModal: boolean;
    qrModalOptions?: QrModalOptions;
}
export interface ConnectOps {
    chains?: number[];
    optionalChains?: number[];
    rpcMap?: EthereumRpcMap;
    pairingTopic?: string;
}
export declare type AuthenticateParams = {
    chains?: number[];
} & Omit<AuthTypes.SessionAuthenticateParams, "chains">;
export interface IEthereumProvider extends IProvider {
    connect(opts?: ConnectOps | undefined): Promise<void>;
}
export declare function getRpcUrl(chainId: string, rpc: EthereumRpcConfig): string | undefined;
export declare function getEthereumChainId(chains: string[]): number;
export declare function toHexChainId(chainId: number): string;
export declare type NamespacesParams = {
    chains: EthereumRpcConfig["chains"];
    optionalChains: EthereumRpcConfig["optionalChains"];
    methods?: EthereumRpcConfig["methods"];
    optionalMethods?: EthereumRpcConfig["methods"];
    events?: EthereumRpcConfig["events"];
    rpcMap: EthereumRpcConfig["rpcMap"];
    optionalEvents?: EthereumRpcConfig["events"];
};
export declare function buildNamespaces(params: NamespacesParams): {
    required?: Namespace;
    optional?: Namespace;
};
declare type ArrayOneOrMore<T> = {
    0: T;
} & Array<T>;
export declare type ChainsProps = {
    chains: ArrayOneOrMore<number>;
    optionalChains?: number[];
} | {
    chains?: number[];
    optionalChains: ArrayOneOrMore<number>;
};
export declare type EthereumProviderOptions = {
    projectId: string;
    methods?: string[];
    optionalMethods?: string[];
    events?: string[];
    optionalEvents?: string[];
    rpcMap?: EthereumRpcMap;
    metadata?: Metadata;
    showQrModal: boolean;
    qrModalOptions?: QrModalOptions;
    disableProviderPing?: boolean;
    relayUrl?: string;
    storageOptions?: KeyValueStorageOptions;
} & ChainsProps & UniversalProviderOpts;
export declare class EthereumProvider implements IEthereumProvider {
    events: EventEmitter;
    namespace: string;
    accounts: string[];
    signer: InstanceType<typeof UniversalProvider>;
    chainId: number;
    modal?: any;
    protected rpc: EthereumRpcConfig;
    protected readonly STORAGE_KEY: string;
    constructor();
    static init(opts: EthereumProviderOptions): Promise<EthereumProvider>;
    request<T = unknown>(args: RequestArguments, expiry?: number): Promise<T>;
    sendAsync(args: RequestArguments, callback: (error: Error | null, response: JsonRpcResult) => void, expiry?: number): void;
    get connected(): boolean;
    get connecting(): boolean;
    enable(): Promise<ProviderAccounts>;
    connect(opts?: ConnectOps): Promise<void>;
    authenticate(params: AuthenticateParams, walletUniversalLink?: string): Promise<AuthTypes.AuthenticateResponseResult | undefined>;
    disconnect(): Promise<void>;
    on: IEthereumProviderEvents["on"];
    once: IEthereumProviderEvents["once"];
    removeListener: IEthereumProviderEvents["removeListener"];
    off: IEthereumProviderEvents["off"];
    get isWalletConnect(): boolean;
    get session(): SessionTypes.Struct | undefined;
    protected registerEventListeners(): void;
    protected switchEthereumChain(chainId: number): void;
    protected isCompatibleChainId(chainId: string): boolean;
    protected formatChainId(chainId: number): string;
    protected parseChainId(chainId: string): number;
    protected setChainIds(chains: string[]): void;
    protected setChainId(chain: string): void;
    protected parseAccountId(account: string): {
        chainId: string;
        address: string;
    };
    protected setAccounts(accounts: string[]): void;
    protected getRpcConfig(opts: EthereumProviderOptions): EthereumRpcConfig;
    protected buildRpcMap(chains: number[], projectId: string): EthereumRpcMap;
    protected initialize(opts: EthereumProviderOptions): Promise<void>;
    protected loadConnectOpts(opts?: ConnectOps): void;
    protected getRpcUrl(chainId: number, projectId?: string): string;
    protected loadPersistedSession(): Promise<void>;
    protected reset(): void;
    protected persist(): void;
    protected parseAccounts(payload: string | string[]): string[];
    protected parseAccount: (payload: any) => string;
}
export default EthereumProvider;
//# sourceMappingURL=EthereumProvider.d.ts.map