import type { BaseTransactionOptions } from "../../../transaction/types.js";
import { type RoleInput } from "../utils.js";
export { isGetRoleAdminSupported } from "../__generated__/IPermissions/read/getRoleAdmin.js";
/**
 * @extension PERMISSIONS
 */
export type GetRoleAdminParams = {
    role: RoleInput;
};
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
export declare function getRoleAdmin(options: BaseTransactionOptions<GetRoleAdminParams>): Promise<string>;
//# sourceMappingURL=getRoleAdmin.d.ts.map