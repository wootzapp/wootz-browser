import type { Address } from "abitype";
import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * Represents the parameters for the `registerFid` function.
 * @extension FARCASTER
 */
export type RegisterFidParams = {
    client: ThirdwebClient;
    recoveryAddress: Address;
    chain?: Chain;
    extraStorage?: bigint | string | number;
    disableCache?: boolean;
};
/**
 * Registers a Farcaster fid for the given wallet.
 * @param options - The options for registering an account.
 * @returns A prepared transaction object to register the account.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { registerFid } from "thirdweb/extensions/farcaster";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = registerFid({
 *  client,
 * 	recoveryAddress
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function registerFid(options: RegisterFidParams): import("../../../transaction/prepare-transaction.js").PreparedTransaction<[], import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=registerFid.d.ts.map