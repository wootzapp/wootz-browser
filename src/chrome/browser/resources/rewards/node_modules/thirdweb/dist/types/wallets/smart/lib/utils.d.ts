import type { UserOperationV06, UserOperationV06Hexed, UserOperationV07, UserOperationV07Hexed } from "../types.js";
export declare const generateRandomUint192: () => bigint;
/**
 * @internal
 */
export declare function hexlifyUserOp(userOp: UserOperationV06 | UserOperationV07): UserOperationV06Hexed | UserOperationV07Hexed;
//# sourceMappingURL=utils.d.ts.map