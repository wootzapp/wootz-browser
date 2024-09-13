"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isGetRoleMemberCountSupported = void 0;
exports.getRoleMemberCount = getRoleMemberCount;
const getRoleMemberCount_js_1 = require("../__generated__/IPermissionsEnumerable/read/getRoleMemberCount.js");
const utils_js_1 = require("../utils.js");
var getRoleMemberCount_js_2 = require("../__generated__/IPermissionsEnumerable/read/getRoleMemberCount.js");
Object.defineProperty(exports, "isGetRoleMemberCountSupported", { enumerable: true, get: function () { return getRoleMemberCount_js_2.isGetRoleMemberCountSupported; } });
/**
 * Retrieves the number of members of a specific role.
 *
 * @param options - The options for retrieving role member count.
 * @returns A promise that resolves to the number of members of the role.
 * @extension PERMISSIONS
 *
 * @example
 * ```ts
 * import { getRoleMemberCount } from "thirdweb/extensions/permissions";
 *
 * const result = await getRoleMemberCount({
 *  contract,
 *  role: "admin",
 * });
 * ```
 */
function getRoleMemberCount(options) {
    return (0, getRoleMemberCount_js_1.getRoleMemberCount)({
        contract: options.contract,
        role: (0, utils_js_1.getRoleHash)(options.role),
    });
}
//# sourceMappingURL=getRoleMemberCount.js.map