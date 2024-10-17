"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getOffer = getOffer;
const eth_getBlockByNumber_js_1 = require("../../../../rpc/actions/eth_getBlockByNumber.js");
const rpc_js_1 = require("../../../../rpc/rpc.js");
const getOffer_js_1 = require("../../__generated__/IOffers/read/getOffer.js");
const utils_js_1 = require("../utils.js");
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
async function getOffer(options) {
    const rpcClient = (0, rpc_js_1.getRpcClient)(options.contract);
    const [rawOffer, latestBlock] = await Promise.all([
        (0, getOffer_js_1.getOffer)(options),
        (0, eth_getBlockByNumber_js_1.eth_getBlockByNumber)(rpcClient, {
            blockTag: "latest",
        }),
    ]);
    return (0, utils_js_1.mapOffer)({
        contract: options.contract,
        latestBlock,
        rawOffer,
    });
}
//# sourceMappingURL=getOffer.js.map