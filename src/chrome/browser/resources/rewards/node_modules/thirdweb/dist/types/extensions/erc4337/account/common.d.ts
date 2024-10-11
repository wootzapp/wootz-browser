import type { ThirdwebContract } from "../../../contract/contract.js";
import type { Account } from "../../../wallets/interfaces/wallet.js";
import type { SetPermissionsForSignerParams } from "../__generated__/IAccountPermissions/write/setPermissionsForSigner.js";
import { type AccountPermissions } from "./types.js";
/**
 * @internal
 */
export declare function signPermissionRequest(options: {
    account: Account;
    contract: ThirdwebContract;
    req: SetPermissionsForSignerParams["req"];
}): Promise<{
    req: {
        signer: string;
        isAdmin: number;
        approvedTargets: readonly string[];
        nativeTokenLimitPerTransaction: bigint;
        permissionStartTimestamp: bigint;
        permissionEndTimestamp: bigint;
        reqValidityStartTimestamp: bigint;
        reqValidityEndTimestamp: bigint;
        uid: `0x${string}`;
    };
    signature: `0x${string}`;
}>;
/**
 * @internal
 */
export declare function toContractPermissions(options: {
    target: string;
    permissions: AccountPermissions;
}): Promise<SetPermissionsForSignerParams["req"]>;
/**
 * @internal
 */
export declare function defaultPermissionsForAdmin(options: {
    target: string;
    action: "add-admin" | "remove-admin";
}): Promise<SetPermissionsForSignerParams["req"]>;
//# sourceMappingURL=common.d.ts.map