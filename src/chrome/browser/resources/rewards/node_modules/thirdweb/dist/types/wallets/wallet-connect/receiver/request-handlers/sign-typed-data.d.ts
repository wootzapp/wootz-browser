import type { Hex } from "../../../../utils/encoding/hex.js";
import type { Account } from "../../../interfaces/wallet.js";
import type { WalletConnectSignTypedDataRequestParams } from "../types.js";
/**
 * @internal
 */
export declare function handleSignTypedDataRequest(options: {
    account: Account;
    params: WalletConnectSignTypedDataRequestParams;
}): Promise<Hex>;
//# sourceMappingURL=sign-typed-data.d.ts.map