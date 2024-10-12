import type { ThirdwebClient } from "../../../../client/client.js";
import type { WalletId } from "../../../../wallets/wallet-types.js";
/**
 * @internal
 */
export declare const ScanScreen: React.FC<{
    onBack?: () => void;
    onGetStarted?: () => void;
    qrCodeUri?: string;
    walletName: string;
    walletId: WalletId;
    qrScanInstruction: string;
    getStartedLink: string;
    error: boolean;
    onRetry: () => void;
    connectModalSize: "compact" | "wide";
    client: ThirdwebClient;
}>;
//# sourceMappingURL=ScanScreen.d.ts.map