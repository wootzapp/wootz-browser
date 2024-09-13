"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isGetRoleAdminSupported = void 0;
exports.getRoleAdmin = getRoleAdmin;
const getRoleAdmin_js_1 = require("../__generated__/IPermissions/read/getRoleAdmin.js");
const utils_js_1 = require("../utils.js");
var getRoleAdmin_js_2 = require("../__generated__/IPermissions/read/getRoleAdmin.js");
Object.defineProperty(exports, "isGetRoleAdminSupported", { enumerable: true, get: function () { return getRoleAdmin_js_2.isGetRoleAdminSupported; } });
/**
 * Gets the admin of a role.
 *
 * @param options - The options for getting the role's admin.
 * @returns The address of the role's admin.
 * @extension PERMISSIONS
 * @example
 * ```ts
 * import { getRoleAdmin } from "thirdweb/extensions/permissions";
 *
 * const result = await getRoleAdmin({
 *  contract,
 *  role: "admin",
 * });
 */
function getRoleAdmin(options) {
    return (0, getRoleAdmin_js_1.getRoleAdmin)({
        contract: options.contract,
        role: (0, utils_js_1.getRoleHash)(options.role),
    });
}
//# sourceMappingURL=getRoleAdmin.js.map