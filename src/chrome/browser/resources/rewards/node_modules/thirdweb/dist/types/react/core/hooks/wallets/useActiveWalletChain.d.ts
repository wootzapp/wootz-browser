/**
 * A hook that returns the chain the active wallet is connected to
 * @returns The chain the active wallet is connected to or null if no active wallet.
 * @example
 * ```jsx
 * import { useActiveWalletChain } from "thirdweb/react";
 *
 * const activeChain = useActiveWalletChain();
 * ```
 * @walletConnection
 */
export declare function useActiveWalletChain(): Readonly<import("../../../../chains/types.js").ChainOptions & {
    rpc: string;
}> | undefined;
//# sourceMappingURL=useActiveWalletChain.d.ts.map