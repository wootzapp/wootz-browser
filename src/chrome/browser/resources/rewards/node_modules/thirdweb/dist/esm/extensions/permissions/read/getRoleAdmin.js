import { getRoleAdmin as generatedGetRoleAdmin } from "../__generated__/IPermissions/read/getRoleAdmin.js";
import { getRoleHash } from "../utils.js";
export { isGetRoleAdminSupported } from "../__generated__/IPermissions/read/getRoleAdmin.js";
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
export function getRoleAdmin(options) {
    return generatedGetRoleAdmin({
        contract: options.contract,
        role: getRoleHash(options.role),
    });
}
//# sourceMappingURL=getRoleAdmin.js.map