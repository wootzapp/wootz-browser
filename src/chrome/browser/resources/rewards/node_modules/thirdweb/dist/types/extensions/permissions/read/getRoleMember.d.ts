import type { BaseTransactionOptions } from "../../../transaction/types.js";
import { type RoleInput } from "../utils.js";
export { isGetRoleMemberSupported } from "../__generated__/IPermissionsEnumerable/read/getRoleMember.js";
/**
 * @extension PERMISSIONS
 */
export type GetRoleMemberParams = {
    role: RoleInput;
    index: bigint;
};
/**
 * Retrieves a specific member of a specific role.
 *
 * @param options - The options for retrieving the role member.
 * @returns A promise that resolves to the address of the role member.
 * @extension PERMISSIONS
 *
 * @example
 * ```ts
 * import { getRoleMember } from "thirdweb/extensions/permissions";
 *
 * const address = await getRoleMember({
 *  contract,
 *  role: "admin",
 *  index: 0n,
 * });
 * ```
 */
export declare function getRoleMember(options: BaseTransactionOptions<GetRoleMemberParams>): Promise<string>;
//# sourceMappingURL=getRoleMember.d.ts.map