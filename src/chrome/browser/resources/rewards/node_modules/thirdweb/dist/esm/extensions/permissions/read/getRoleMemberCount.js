import { getRoleMemberCount as generatedGetRoleMemberCount } from "../__generated__/IPermissionsEnumerable/read/getRoleMemberCount.js";
import { getRoleHash } from "../utils.js";
export { isGetRoleMemberCountSupported } from "../__generated__/IPermissionsEnumerable/read/getRoleMemberCount.js";
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
export function getRoleMemberCount(options) {
    return generatedGetRoleMemberCount({
        contract: options.contract,
        role: getRoleHash(options.role),
    });
}
//# sourceMappingURL=getRoleMemberCount.js.map