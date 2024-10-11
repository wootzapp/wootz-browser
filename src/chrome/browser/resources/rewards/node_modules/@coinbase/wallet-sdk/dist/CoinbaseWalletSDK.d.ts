import { LogoType } from './assets/wallet-logo';
import { AppMetadata, Preference, ProviderInterface } from './core/provider/interface';
type CoinbaseWalletSDKOptions = Partial<AppMetadata>;
export declare class CoinbaseWalletSDK {
    private metadata;
    constructor(metadata: Readonly<CoinbaseWalletSDKOptions>);
    makeWeb3Provider(preference?: Preference): ProviderInterface;
    /**
     * Official Coinbase Wallet logo for developers to use on their frontend
     * @param type Type of wallet logo: "standard" | "circle" | "text" | "textWithLogo" | "textLight" | "textWithLogoLight"
     * @param width Width of the logo (Optional)
     * @returns SVG Data URI
     */
    getCoinbaseWalletLogo(type: LogoType, width?: number): string;
    private storeLatestVersion;
}
export {};
