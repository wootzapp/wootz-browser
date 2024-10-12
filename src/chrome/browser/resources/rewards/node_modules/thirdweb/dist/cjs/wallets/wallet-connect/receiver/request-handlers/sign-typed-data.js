"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.handleSignTypedDataRequest = handleSignTypedDataRequest;
const utils_js_1 = require("../utils.js");
/**
 * @internal
 */
async function handleSignTypedDataRequest(options) {
    const { account, params } = options;
    (0, utils_js_1.validateAccountAddress)(account, params[0]);
    return account.signTypedData(
    // The data could be sent to us as a string or object, depending on the level of parsing on the client side
    typeof params[1] === "string" ? JSON.parse(params[1]) : params[1]);
}
//# sourceMappingURL=sign-typed-data.js.map