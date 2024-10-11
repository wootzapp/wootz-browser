"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.resolveAddress = resolveAddress;
const addresses_js_1 = require("../../../constants/addresses.js");
const address_js_1 = require("../../../utils/address.js");
const withCache_js_1 = require("../../../utils/promise/withCache.js");
/**
 * Take in a Lens handle or local-name and return the wallet address behind that handle/local-name.
 * For example, "lens/vitalik" is a handle, with "lens" being the namespace and "vitalik" being the local name
 * @extension LENS
 * @example
 * ```ts
 * import { resolveAddress } from "thirdweb/extensions/lens";
 *
 * const walletAddress = await resolveAddress({
 *   name: "vitalik",
 *   client,
 * });
 * ```
 */
async function resolveAddress(options) {
    const { name, overrides, client } = options;
    if ((0, address_js_1.isAddress)(name)) {
        return (0, address_js_1.getAddress)(name);
    }
    return (0, withCache_js_1.withCache)(async () => {
        const [{ getContract }, { getTokenId }, { ownerOf }, { polygon }, { LENS_HANDLE_ADDRESS },] = await Promise.all([
            Promise.resolve().then(() => require("../../../contract/contract.js")),
            Promise.resolve().then(() => require("../__generated__/LensHandle/read/getTokenId.js")),
            Promise.resolve().then(() => require("../../erc721/__generated__/IERC721A/read/ownerOf.js")),
            Promise.resolve().then(() => require("../../../chains/chain-definitions/polygon.js")),
            Promise.resolve().then(() => require("../consts.js")),
        ]);
        const contract = getContract({
            address: overrides?.lensHandleContractAddress || LENS_HANDLE_ADDRESS,
            chain: overrides?.chain || polygon,
            client,
        });
        /**
         * For better UX, we accept both handle and local name
         * The difference: handle = <namespace>/<local-name>
         * For example, "lens/vitalik" is a handle, with "lens" being the namespace and "vitalik" being the local name
         * Currently there's only 1 namespace called "lens" but we should make sure the code can scale when there are more
         *
         * Since lens does not allow "/" in the name,
         * if the string contains "/", it is either invalid, or it definitely contains the namespace
         * In that case we remove the possible namespace because the `getTokenId` method only accepts localName
         */
        const isPossibleHandle = name.includes("/");
        const localName = isPossibleHandle ? name.split("/")[1] : name;
        if (!localName) {
            throw new Error(`missing local name from ${name}`);
        }
        const tokenId = await getTokenId({ contract, localName });
        if (!tokenId) {
            throw new Error(`Could not retrieve tokenId from localName: ${localName}`);
        }
        /**
         * Another thing to note is that even if you enter an invalid localName,
         * `getTokenId` still returns you a tokenId - so never rely on the result alone.
         * Check if the tokenId truly exists using `exists` or in this case, `ownerOf`
         */
        const address = await ownerOf({ contract, tokenId }).catch(() => addresses_js_1.ZERO_ADDRESS);
        if (address === addresses_js_1.ZERO_ADDRESS) {
            throw new Error(`Could not fetch the wallet address for lens handle: ${name}`);
        }
        return address;
    }, {
        cacheKey: `lens:addr:${name}`,
        // 1min cache
        cacheTime: 60 * 1000,
    });
}
//# sourceMappingURL=resolveAddress.js.map