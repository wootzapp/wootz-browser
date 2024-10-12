import type { BaseTransactionOptions } from "../../../../transaction/types.js";
import { type AcceptOfferParams as GeneratedAcceptOfferParams } from "../../__generated__/IOffers/write/acceptOffer.js";
/**
 * @extension MARKETPLACE
 */
export type AcceptOfferParams = GeneratedAcceptOfferParams;
/**
 * Accepts an offer after performing necessary checks and validations.
 * Throws an error if the offer is not active, the offeror's balance is insufficient,
 * or the offeror's allowance is insufficient.
 *
 * @param options - The options for accepting the offer.
 * @returns A transaction object that can be sent to accept the offer.
 * @throws Error when sending the transaction if the offer is not active, the offeror's balance is insufficient,
 * or the offeror's allowance is insufficient.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { acceptOffer } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const acceptOfferTx = acceptOffer({
 *  contract,
 *  offerId: 1n,
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function acceptOffer(options: BaseTransactionOptions<AcceptOfferParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=acceptOffer.d.ts.map