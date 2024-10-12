import { type ThirdwebContract } from "../../../contract/contract.js";
import type { FarcasterContractOptions } from "./contractOptions.js";
/**
 * Retrieves the IdRegistry contract.
 * @param options - The thirdweb client and an optional custom chain.
 * @returns The IdRegistry contract instance.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getIdRegistry } from "thirdweb/extensions/farcaster";
 *
 * const idRegistry = await getIdRegistry({
 *  client,
 * });
 * ```
 */
export declare function getIdRegistry(options: FarcasterContractOptions): ThirdwebContract;
//# sourceMappingURL=getIdRegistry.d.ts.map