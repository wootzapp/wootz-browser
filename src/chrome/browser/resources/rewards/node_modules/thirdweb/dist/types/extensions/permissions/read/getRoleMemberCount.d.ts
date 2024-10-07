import type { BaseTransactionOptions } from "../../../transaction/types.js";
import { type RoleInput } from "../utils.js";
export { isGetRoleMemberCountSupported } from "../__generated__/IPermissionsEnumerable/read/getRoleMemberCount.js";
/**
 * @extension PERMISSIONS
 */
export type GetRoleMemberCountParams = {
    role: RoleInput;
};
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
export declare function getRoleMemberCount(options: BaseTransactionOptions<GetRoleMemberCountParams>): Promise<bigint>;
//# sourceMappingURL=getRoleMemberCount.d.ts.map