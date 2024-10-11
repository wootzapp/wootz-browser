declare const mapping: {
    readonly handshake: readonly ["eth_requestAccounts"];
    readonly sign: readonly ["eth_ecRecover", "personal_sign", "personal_ecRecover", "eth_signTransaction", "eth_sendTransaction", "eth_signTypedData_v1", "eth_signTypedData_v3", "eth_signTypedData_v4", "eth_signTypedData", "wallet_addEthereumChain", "wallet_switchEthereumChain", "wallet_watchAsset", "wallet_getCapabilities", "wallet_sendCalls", "wallet_showCallsStatus"];
    readonly state: readonly ["eth_chainId", "eth_accounts", "eth_coinbase", "net_version"];
    readonly deprecated: readonly ["eth_sign", "eth_signTypedData_v2"];
    readonly unsupported: readonly ["eth_subscribe", "eth_unsubscribe"];
    readonly fetch: readonly [];
};
export type MethodCategory = keyof typeof mapping;
export type Method<C extends MethodCategory = MethodCategory> = (typeof mapping)[C][number];
export declare function determineMethodCategory(method: string): MethodCategory | undefined;
export {};
