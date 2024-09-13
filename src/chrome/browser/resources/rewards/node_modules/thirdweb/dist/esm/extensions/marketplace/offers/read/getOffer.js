import { eth_getBlockByNumber } from "../../../../rpc/actions/eth_getBlockByNumber.js";
import { getRpcClient } from "../../../../rpc/rpc.js";
import { getOffer as getOfferGenerated, } from "../../__generated__/IOffers/read/getOffer.js";
import { mapOffer } from "../utils.js";
/**
 * Retrieves an offer based on the provided options.
 * @param options - The options for retrieving the offer.
 * @returns A promise that resolves to the offer.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getOffer } from "thirdweb/extensions/marketplace";
 *
 * const listing = await getOffer({ contract, listingId: 1n });
 * ```
 */
export async function getOffer(options) {
    const rpcClient = getRpcClient(options.contract);
    const [rawOffer, latestBlock] = await Promise.all([
        getOfferGenerated(options),
        eth_getBlockByNumber(rpcClient, {
            blockTag: "latest",
        }),
    ]);
    return mapOffer({
        contract: options.contract,
        latestBlock,
        rawOffer,
    });
}
//# sourceMappingURL=getOffer.js.map