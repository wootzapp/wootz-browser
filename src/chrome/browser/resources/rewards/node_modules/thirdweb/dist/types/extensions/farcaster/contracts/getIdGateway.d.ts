import { type ThirdwebContract } from "../../../contract/contract.js";
import type { FarcasterContractOptions } from "./contractOptions.js";
/**
 * Retrieves the IdGateway contract.
 * @param options - The thirdweb client and an optional custom chain.
 * @returns The IdGateway contract instance.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getIdGatway } from "thirdweb/extensions/farcaster";
 *
 * const idGateway = await getIdGateway({
 *  client,
 * });
 * ```
 */
export declare function getIdGateway(options: FarcasterContractOptions): ThirdwebContract;
//# sourceMappingURL=getIdGateway.d.ts.map