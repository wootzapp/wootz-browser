import type { Hex } from "../../../../utils/encoding/hex.js";
import type { Account } from "../../../interfaces/wallet.js";
import type { WalletConnectRawTransactionRequestParams } from "../types.js";
/**
 * @internal
 */
export declare function handleSendRawTransactionRequest(options: {
    account: Account;
    chainId: number;
    params: WalletConnectRawTransactionRequestParams;
}): Promise<Hex>;
//# sourceMappingURL=send-raw-transaction.d.ts.map