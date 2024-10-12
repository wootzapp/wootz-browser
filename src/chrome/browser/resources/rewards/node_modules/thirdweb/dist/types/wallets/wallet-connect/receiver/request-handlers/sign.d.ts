import type { Hex } from "../../../../utils/encoding/hex.js";
import type { Account } from "../../../interfaces/wallet.js";
import type { WalletConnectSignRequestPrams } from "../types.js";
/**
 * @internal
 */
export declare function handleSignRequest(options: {
    account: Account;
    params: WalletConnectSignRequestPrams;
}): Promise<Hex>;
//# sourceMappingURL=sign.d.ts.map