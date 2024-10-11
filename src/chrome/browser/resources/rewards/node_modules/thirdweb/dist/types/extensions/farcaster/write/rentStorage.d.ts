import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
/**
 * Represents the parameters for the `rentStorage` function.
 * @extension FARCASTER
 */
export type RentStorageParams = {
    client: ThirdwebClient;
    fid: bigint | number | string;
    units?: bigint | number | string;
    chain?: Chain;
    disableCache?: boolean;
};
/**
 * Rent storage for the provided farcaster fid.
 * @param options - The options for calling the `rentStorage` function.
 * @returns A prepared transaction object to rent the storage.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { rentStorage } from "thirdweb/extensions/farcaster";
 * import { sendTransaction } from "thirdweb";
 *
 * const transaction = rentStorage({
 *  client,
 * 	fid,
 *  units,
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function rentStorage(options: RentStorageParams): import("../../../transaction/prepare-transaction.js").PreparedTransaction<[], import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=rentStorage.d.ts.map