"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getListing = getListing;
exports.isGetListingSupported = isGetListingSupported;
const eth_getBlockByNumber_js_1 = require("../../../../rpc/actions/eth_getBlockByNumber.js");
const rpc_js_1 = require("../../../../rpc/rpc.js");
const GetListing = require("../../__generated__/IDirectListings/read/getListing.js");
const utils_js_1 = require("../utils.js");
/**
 * Retrieves a direct listing based on the provided options.
 * @param options - The options for retrieving the listing.
 * @returns A promise that resolves to the direct listing.
 * @extension MARKETPLACE
 * @example
 *
 * ```ts
 * import { getListing } from "thirdweb/extensions/marketplace";
 *
 * const listing = await getListing({ contract, listingId: 1n });
 * ```
 */
async function getListing(options) {
    const rpcClient = (0, rpc_js_1.getRpcClient)(options.contract);
    const [rawListing, latestBlock] = await Promise.all([
        GetListing.getListing(options),
        (0, eth_getBlockByNumber_js_1.eth_getBlockByNumber)(rpcClient, {
            blockTag: "latest",
        }),
    ]);
    return (0, utils_js_1.mapDirectListing)({
        contract: options.contract,
        latestBlock,
        rawListing,
    });
}
/**
 * Checks if the `isGetListingSupported` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `isGetListingSupported` method is supported.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { isGetListingSupported } from "thirdweb/extensions/marketplace";
 *
 * const supported = isGetListingSupported(["0x..."]);
 * ```
 */
function isGetListingSupported(availableSelectors) {
    return GetListing.isGetListingSupported(availableSelectors);
}
//# sourceMappingURL=getListing.js.map