"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isERC721 = isERC721;
const supportsInterface_js_1 = require("../../erc165/__generated__/IERC165/read/supportsInterface.js");
/**
 * Check if a contract supports the ERC721 interface.
 * @param options - The transaction options.
 * @returns A boolean indicating whether the contract supports the ERC721 interface.
 * @extension ERC721
 * @example
 * ```ts
 * import { isERC721 } from "thirdweb/extensions/erc721";
 * const result = await isERC721({ contract });
 * ```
 */
function isERC721(options) {
    return (0, supportsInterface_js_1.supportsInterface)({
        contract: options.contract,
        interfaceId: "0x80ac58cd",
    });
}
//# sourceMappingURL=isERC721.js.map