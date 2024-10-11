import type { Chain } from "../../../chains/types.js";
import type { ThirdwebClient } from "../../../client/client.js";
import type { PaymasterResult, UserOperationV06, UserOperationV07 } from "../types.js";
/**
 * Get paymaster and data details for a user operation.
 * @param args - The userOp and options
 * @returns - The paymaster and data details for the user operation.
 * @example
 * ```ts
 * import { getPaymasterAndData } from "thirdweb/wallets/smart";
 *
 * const userOp = createUnsignedUserOp(...);
 *
 * const paymasterAndData = await getPaymasterAndData({
 *  userOp,
 *  client,
 *  chain,
 * });
 * ```
 * @walletUtils
 */
export declare function getPaymasterAndData(args: {
    userOp: UserOperationV06 | UserOperationV07;
    client: ThirdwebClient;
    chain: Chain;
    entrypointAddress?: string;
    paymasterOverride?: (userOp: UserOperationV06 | UserOperationV07) => Promise<PaymasterResult>;
}): Promise<PaymasterResult>;
//# sourceMappingURL=paymaster.d.ts.map