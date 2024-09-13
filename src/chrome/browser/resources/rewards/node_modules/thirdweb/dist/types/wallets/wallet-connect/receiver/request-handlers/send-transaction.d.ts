import type { ThirdwebClient } from "../../../../client/client.js";
import { type Hex } from "../../../../utils/encoding/hex.js";
import type { Account } from "../../../interfaces/wallet.js";
import type { WalletConnectTransactionRequestParams } from "../types.js";
/**
 * @internal
 */
export declare function handleSendTransactionRequest(options: {
    account: Account;
    chainId: number;
    thirdwebClient: ThirdwebClient;
    params: WalletConnectTransactionRequestParams;
}): Promise<Hex>;
//# sourceMappingURL=send-transaction.d.ts.map