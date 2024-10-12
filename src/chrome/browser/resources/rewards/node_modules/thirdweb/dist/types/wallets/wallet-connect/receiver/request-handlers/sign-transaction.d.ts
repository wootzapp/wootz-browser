import { type Hex } from "../../../../utils/encoding/hex.js";
import type { Account } from "../../../interfaces/wallet.js";
import type { WalletConnectTransactionRequestParams } from "../types.js";
/**
 * @internal
 */
export declare function handleSignTransactionRequest(options: {
    account: Account;
    params: WalletConnectTransactionRequestParams;
}): Promise<Hex>;
//# sourceMappingURL=sign-transaction.d.ts.map