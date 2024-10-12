import type { ThirdwebClient } from "../../../../client/client.js";
import type { WalletId } from "../../../../wallets/wallet-types.js";
/**
 * @internal
 */
export declare const ConnectingScreen: React.FC<{
    onBack?: () => void;
    walletId: WalletId;
    walletName: string;
    onGetStarted?: () => void;
    errorConnecting: boolean;
    onRetry: () => void;
    locale: {
        getStartedLink: string;
        tryAgain: string;
        instruction: string;
        failed: string;
        inProgress: string;
    };
    client: ThirdwebClient;
    size: "compact" | "wide";
}>;
//# sourceMappingURL=ConnectingScreen.d.ts.map