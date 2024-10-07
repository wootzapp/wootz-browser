/**
 * Switch to blockchain with given chain id in the active wallet.
 * @returns A function to switch to blockchain with given chain id in the active wallet.
 * @example
 * ```jsx
 * import { useSwitchActiveWalletChain } from "thirdweb/react";
 * import { sepolia } from "thirdweb/chains";
 *
 * const switchChain = useSwitchActiveWalletChain();
 *
 * // later in your code
 * <button onClick={() => switchChain(sepolia)}>Switch Chain</button>
 * ```
 * @walletConnection
 */
export declare function useSwitchActiveWalletChain(): (chain: import("../../../../chains/types.js").Chain) => Promise<void>;
//# sourceMappingURL=useSwitchActiveWalletChain.d.ts.map