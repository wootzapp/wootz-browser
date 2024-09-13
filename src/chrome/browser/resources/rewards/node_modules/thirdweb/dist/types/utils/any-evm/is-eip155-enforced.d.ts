import type { Chain } from "../../chains/types.js";
import type { ThirdwebClient } from "../../client/client.js";
type IsEIP155EnforcedOptions = {
    chain: Chain;
    client: ThirdwebClient;
};
/**
 * Checks whether EIP-155 is enforced by sending a random transaction of legacy type (pre-EIP-155)
 * and parsing the error message.
 * @param options - The options for checking EIP-155 enforcement.
 * @returns A promise that resolves to a boolean indicating whether EIP-155 is enforced.
 * @example
 * ```ts
 * import { isEIP155Enforced } from "thirdweb/utils";
 * const isEIP155 = await isEIP155Enforced({ chain, client });
 * ```
 * @utils
 */
export declare function isEIP155Enforced(options: IsEIP155EnforcedOptions): Promise<boolean>;
export {};
//# sourceMappingURL=is-eip155-enforced.d.ts.map