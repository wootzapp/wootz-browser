import type { Chain } from "../chains/types.js";
import type { ThirdwebClient } from "../client/client.js";
import type { Account, Wallet } from "../wallets/interfaces/wallet.js";
export type AdapterWalletOptions = {
    client: ThirdwebClient;
    adaptedAccount: Account;
    chain: Chain;
    onDisconnect: () => Promise<void> | void;
    switchChain: (chain: Chain) => Promise<void> | void;
};
/**
 * Creates a wallet from the given account.
 *
 * You can use this to:
 *
 * - convert a third party library wallet (wagmi, viem, ethers) into a thirdweb wallet.
 * - connect with a private key (for automated tests)
 *
 * Available wallet adatpers:
 * - [Viem](https://portal.thirdweb.com/references/typescript/v5/viemAdapter)
 * - [Ethers 6](https://portal.thirdweb.com/references/typescript/v5/ethers6Adapter)
 * - [Ethers 5](https://portal.thirdweb.com/references/typescript/v5/ethers5Adapter)
 *
 * @param options - The options for the adapter wallet.
 * @returns a wallet instance.
 *
 * @example
 * ```ts
 * import { createWalletAdapter } from "thirdweb";
 *
 * const wallet = createWalletAdapter({
 *  client,
 *  adaptedAccount,
 *  chain,
 *  onDisconnect: () => {
 *    // disconnect logic
 *  },
 *  switchChain: async (chain) => {
 *    // switch chain logic
 *  },
 * });
 * ```
 * @wallet
 */
export declare function createWalletAdapter(options: AdapterWalletOptions): Wallet<"adapter">;
//# sourceMappingURL=wallet-adapter.d.ts.map