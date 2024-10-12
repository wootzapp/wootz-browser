"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getHandleFromProfileId = getHandleFromProfileId;
const polygon_js_1 = require("../../../chains/chain-definitions/polygon.js");
const contract_js_1 = require("../../../contract/contract.js");
const getHandle_js_1 = require("../__generated__/LensHandle/read/getHandle.js");
const getDefaultHandle_js_1 = require("../__generated__/TokenHandleRegistry/read/getDefaultHandle.js");
const consts_js_1 = require("../consts.js");
/**
 * Return the Lens handle of a profile in the format: lens/@<name-of-user>
 * @param options
 * @returns
 * @extension LENS
 *
 * @example
 * ```ts
 * import { getHandleFromProfileId } from "thirdweb/extensions/lens";
 *
 * const profileId = 461662n;
 * const handle = await getHandleFromProfileId({ profileId, client }); // "lens/@captain_jack"
 * ```
 */
async function getHandleFromProfileId(options) {
    const { profileId, overrides, client } = options;
    const lensHandleContract = (0, contract_js_1.getContract)({
        address: overrides?.lensHandleAddress || consts_js_1.LENS_HANDLE_ADDRESS,
        chain: overrides?.chain || polygon_js_1.polygon,
        client,
    });
    const tokenHandleRegistryContract = (0, contract_js_1.getContract)({
        address: overrides?.tokenHandleRegistryAddress ||
            consts_js_1.LENS_TOKEN_HANDLE_REGISTRY_ADDRESS,
        chain: overrides?.chain || polygon_js_1.polygon,
        client,
    });
    const handleTokenId = await (0, getDefaultHandle_js_1.getDefaultHandle)({
        contract: tokenHandleRegistryContract,
        profileId,
    }).catch(() => null);
    if (handleTokenId === null) {
        return null;
    }
    // e.g: "lens/@JuanWick"
    const handle = await (0, getHandle_js_1.getHandle)({
        contract: lensHandleContract,
        tokenId: handleTokenId,
    }).catch(() => null);
    return handle;
}
//# sourceMappingURL=getHandleFromProfileId.js.map