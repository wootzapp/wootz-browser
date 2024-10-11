"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.handleSignRequest = handleSignRequest;
const utils_js_1 = require("../utils.js");
/**
 * @internal
 */
async function handleSignRequest(options) {
    const { account, params } = options;
    (0, utils_js_1.validateAccountAddress)(account, params[1]);
    return account.signMessage({ message: { raw: params[0] } });
}
//# sourceMappingURL=sign.js.map