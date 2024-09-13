import type { Chain } from "../../../../chains/types.js";
import type { ThirdwebClient } from "../../../../client/client.js";
import type { WCSupportedWalletIds } from "../../../../wallets/__generated__/wallet-ids.js";
import type { Wallet } from "../../../../wallets/interfaces/wallet.js";
import type { WalletInfo } from "../../../../wallets/wallet-info.js";
import type { InjectedWalletLocale } from "../injected/locale/types.js";
/**
 * QR Scan UI for connecting a specific wallet on desktop.
 * shows a "Connecting" screen and opens the app on mobile.
 * @internal
 */
export declare const WalletConnectConnection: React.FC<{
    onBack?: () => void;
    onGetStarted: () => void;
    locale: InjectedWalletLocale;
    wallet: Wallet<WCSupportedWalletIds>;
    walletInfo: WalletInfo;
    done: () => void;
    chain: Chain | undefined;
    chains: Chain[] | undefined;
    client: ThirdwebClient;
    walletConnect: {
        projectId?: string;
    } | undefined;
    size: "compact" | "wide";
}>;
/**
 * QR Scan UI for connecting a specific wallet on desktop.
 * shows a "Connecting" screen and opens the app on mobile.
 * @internal
 */
export declare const WalletConnectStandaloneConnection: React.FC<{
    onBack?: () => void;
    locale: InjectedWalletLocale;
    wallet: Wallet<"walletConnect">;
    walletInfo: WalletInfo;
    done: () => void;
    setModalVisibility: (value: boolean) => void;
    chain: Chain | undefined;
    chains: Chain[] | undefined;
    client: ThirdwebClient;
    walletConnect: {
        projectId?: string;
    } | undefined;
    size: "compact" | "wide";
}>;
//# sourceMappingURL=WalletConnectConnection.d.ts.map