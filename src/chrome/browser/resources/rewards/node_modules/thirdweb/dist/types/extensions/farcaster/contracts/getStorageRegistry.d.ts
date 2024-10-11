import { type ThirdwebContract } from "../../../contract/contract.js";
import type { FarcasterContractOptions } from "./contractOptions.js";
/**
 * Retrieves the StorageRegistry contract.
 * @param options - The thirdweb client and an optional custom chain.
 * @returns The StorageRegistry contract instance.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getStorageRegistry } from "thirdweb/extensions/farcaster";
 *
 * const storageRegistry = await getStorageRegistry({
 *  client,
 * });
 * ```
 */
export declare function getStorageRegistry(options: FarcasterContractOptions): ThirdwebContract;
//# sourceMappingURL=getStorageRegistry.d.ts.map