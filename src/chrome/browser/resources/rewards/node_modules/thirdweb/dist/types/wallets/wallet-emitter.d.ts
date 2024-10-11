import type { Chain } from "../chains/types.js";
import { type Emitter } from "../utils/tiny-emitter.js";
import type { Account } from "./interfaces/wallet.js";
import type { WalletAutoConnectionOption, WalletId } from "./wallet-types.js";
export type WalletEmitterEvents<TWalletId extends WalletId> = {
    accountChanged: Account;
    accountsChanged: string[];
    disconnect?: never;
    chainChanged: Chain;
    onConnect: WalletAutoConnectionOption<TWalletId>;
};
export type WalletEmitter<TWalletId extends WalletId> = Emitter<WalletEmitterEvents<TWalletId>>;
/**

 * @internal
 */
export declare function createWalletEmitter<const TWalletId extends WalletId>(): Emitter<WalletEmitterEvents<TWalletId>>;
//# sourceMappingURL=wallet-emitter.d.ts.map