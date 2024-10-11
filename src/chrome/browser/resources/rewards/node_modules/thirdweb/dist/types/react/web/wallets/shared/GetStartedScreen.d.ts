import type { ThirdwebClient } from "../../../../client/client.js";
import type { Wallet } from "../../../../wallets/interfaces/wallet.js";
import type { WalletInfo } from "../../../../wallets/wallet-info.js";
import type { InjectedWalletLocale } from "../injected/locale/types.js";
/**
 * @internal
 */
export declare const GetStartedScreen: React.FC<{
    onBack?: () => void;
    wallet: Wallet;
    walletInfo: WalletInfo;
    header?: React.ReactNode;
    footer?: React.ReactNode;
    showBack?: boolean;
    locale: InjectedWalletLocale;
    client: ThirdwebClient;
}>;
//# sourceMappingURL=GetStartedScreen.d.ts.map