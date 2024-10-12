import Web3Provider from "@walletconnect/web3-provider";
import { IWalletConnectSDKOptions, IConnector, ICreateSessionOptions, IWalletConnectProviderOptions, IWCRpcConnectionOptions, IWalletConnectStarkwareProviderOptions } from "@walletconnect/types";
declare class WalletConnectSDK {
    private options?;
    connector: IConnector | undefined;
    constructor(options?: IWalletConnectSDKOptions | undefined);
    get connected(): boolean;
    connect(createSessionOpts?: ICreateSessionOptions): Promise<IConnector>;
    getWeb3Provider(opts?: IWalletConnectProviderOptions): Web3Provider;
    getChannelProvider(opts?: IWCRpcConnectionOptions): void;
    getStarkwareProvider(opts: IWalletConnectStarkwareProviderOptions): void;
    getThreeIdProvider(opts?: IWCRpcConnectionOptions): void;
}
export default WalletConnectSDK;
//# sourceMappingURL=index.d.ts.map