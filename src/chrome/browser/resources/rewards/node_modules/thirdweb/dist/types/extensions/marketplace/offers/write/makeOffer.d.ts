import type { Address } from "abitype";
import type { BaseTransactionOptions } from "../../../../transaction/types.js";
/**
 * @extension MARKETPLACE
 */
export type MakeOfferParams = {
    /**
     * The address of the asset contract to offer on.
     */
    assetContractAddress: Address;
    /**
     * The ID of the token to make an offer for.
     */
    tokenId: bigint;
    /**
     * The address of the currency contract to make an offer with. (ERC20 or native token address)
     */
    currencyContractAddress: Address;
    /**
     * The datetime when the offer expires.
     */
    offerExpiresAt: Date;
    /**
     * The quantity of the asset to make an offer for.
     * @default 1
     */
    quantity?: bigint;
} & ({
    /**
     * The total offer amount for the NFT(s) in wei.
     */
    totalOfferWei: bigint;
} | {
    /**
     * The total offer amount for the NFT(s) in the currency's "Ether" unit
     */
    totalOffer: string;
});
/**
 * Makes an offer for any asset (ERC721 or ERC1155).
 *
 * @param options - The options for making the offer.
 * @returns A transaction object that can be sent to make the offer.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { makeOffer } from "thirdweb/extensions/marketplace";
 * import { sendTransaction } from "thirdweb";
 *
 * const offerTx = makeOffer({
 *  contract,
 *  assetContractAddress: "0x1234567890123456789012345678901234567890",
 *  tokenId: 1n,
 *  currencyContractAddress: "0x1234567890123456789012345678901234567890",
 *  offerExpiresAt: new Date(Date.now() + 1000 * 60 * 60 * 24),
 *  totalOffer: "1.0",
 * });
 *
 * await sendTransaction({ transaction, account });
 * ```
 */
export declare function makeOffer(options: BaseTransactionOptions<MakeOfferParams>): import("../../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=makeOffer.d.ts.map