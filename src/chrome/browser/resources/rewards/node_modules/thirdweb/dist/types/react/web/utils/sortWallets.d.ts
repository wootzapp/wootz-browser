import type { WalletId } from "../../../wallets/wallet-types.js";
/**
 *
 * @internal
 */
export declare function sortWallets<T extends {
    id: string;
}>(wallets: T[], recommendedWallets?: {
    id: WalletId;
}[]): T[];
//# sourceMappingURL=sortWallets.d.ts.map