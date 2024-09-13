import type { Wallet } from "../interfaces/wallet.js";
import type { EcosystemWalletId } from "../wallet-types.js";
export declare function isEcosystemWallet(wallet: Wallet): wallet is Wallet<EcosystemWalletId>;
export declare function isEcosystemWallet(wallet: string): wallet is EcosystemWalletId;
//# sourceMappingURL=is-ecosystem-wallet.d.ts.map