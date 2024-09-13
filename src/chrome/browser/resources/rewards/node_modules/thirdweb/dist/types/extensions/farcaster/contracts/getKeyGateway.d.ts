import { type ThirdwebContract } from "../../../contract/contract.js";
import type { FarcasterContractOptions } from "./contractOptions.js";
/**
 * Retrieves the KeyGateway contract.
 * @param options - The thirdweb client and an optional custom chain.
 * @returns The KeyGateway contract instance.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getKeyGateway } from "thirdweb/extensions/farcaster";
 *
 * const keyGateway = await getKeyGateway({
 *  client,
 * });
 * ```
 */
export declare function getKeyGateway(options: FarcasterContractOptions): ThirdwebContract;
//# sourceMappingURL=getKeyGateway.d.ts.map