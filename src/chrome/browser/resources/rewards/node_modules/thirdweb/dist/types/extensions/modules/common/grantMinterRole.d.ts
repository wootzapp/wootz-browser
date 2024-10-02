import type { AbiParameterToPrimitiveType } from "abitype";
import type { BaseTransactionOptions } from "../../../transaction/types.js";
export type GrantMinterRoleParams = {
    user: AbiParameterToPrimitiveType<{
        name: "user";
        type: "address";
        internalType: "address";
    }>;
};
/**
 * Grants the minter role to a user.
 * @param options - The transaction options.
 * @returns The transaction to send.
 * @modules
 *
 * @example
 * ```ts
 * import { grantMinterRole } from "thirdweb/modules";
 *
 * const tx = await grantMinterRole({
 *   contract,
 *   user: userAddress,
 * });
 * ```
 */
export declare function grantMinterRole(options: BaseTransactionOptions<GrantMinterRoleParams>): import("../../../transaction/prepare-transaction.js").PreparedTransaction<any, import("abitype").AbiFunction, import("../../../transaction/prepare-transaction.js").PrepareTransactionOptions>;
//# sourceMappingURL=grantMinterRole.d.ts.map