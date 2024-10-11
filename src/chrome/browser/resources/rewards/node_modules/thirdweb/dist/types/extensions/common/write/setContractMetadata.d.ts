import type { BaseTransactionOptions } from "../../../transaction/types.js";
export { isSetContractURISupported as isSetContractMetadataSupported } from "../__generated__/IContractMetadata/write/setContractURI.js";
/**
 * @extension COMMON
 */
export type SetContractMetadataParams = Record<string, unknown>;
/**
 * Sets the metadata for a contract.
 *
 * @param options - The options for setting the contract metadata.
 * @returns - The prepared transaction to set the contract metadata.
 * @extension COMMON
 * @example
 * ```ts
 * import { setContractMetadata } from '@thirdweb/extensions/common';
 * import { sendTransaction } from 'thirdweb';
 *
 * const transaction = setContractMetadata({
 *  contract,
 *  name: 'My NFT',
 *  symbol: 'NFT',
 * });
 *
 * // Send the transaction
 * await sendTransaction({
 *  transaction,
 *  account,
 * });
 * ```
 */
export declare function setContractMetadata({ contract, ...restParams }: BaseTransactionOptions<SetContractMetadataParams>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=setContractMetadata.d.ts.map