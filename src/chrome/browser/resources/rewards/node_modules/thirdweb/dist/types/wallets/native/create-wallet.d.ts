import type { Wallet } from "../interfaces/wallet.js";
import type { CreateWalletArgs, WalletId } from "../wallet-types.js";
/**
 * Creates a wallet based on the provided ID and arguments.
 * @param args - The arguments for creating the wallet.
 * @returns - The created wallet.
 * @example
 * ```ts
 * import { createWallet } from "thirdweb/wallets";
 *
 * const metamaskWallet = createWallet("io.metamask");
 *
 * const account = await metamaskWallet.connect({
 *  client,
 * });
 * ```
 * @wallet
 */
export declare function createWallet<const ID extends WalletId>(...args: CreateWalletArgs<ID>): Wallet<ID>;
/**
 * Creates a wallet that allows connecting to any wallet that supports the WalletConnect protocol.
 * @returns The created smart wallet.
 * @example
 * ```ts
 * import { walletConnect } from "thirdweb/wallets";
 *
 * const wallet = walletConnect();
 *
 * const account = await wallet.connect({
 *  client
 * });
 * ```
 * @wallet
 */
export declare function walletConnect(): Wallet<"walletConnect">;
//# sourceMappingURL=create-wallet.d.ts.map