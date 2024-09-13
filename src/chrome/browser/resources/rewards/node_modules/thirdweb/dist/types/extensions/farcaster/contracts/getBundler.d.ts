import { type ThirdwebContract } from "../../../contract/contract.js";
import type { FarcasterContractOptions } from "./contractOptions.js";
/**
 * Retrieves the Bundler contract.
 * @param options - The thirdweb client and an optional custom chain.
 * @returns The Bundler contract instance.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getBundler } from "thirdweb/extensions/farcaster";
 *
 * const bundler = await getBundler({
 *  client,
 * });
 * ```
 */
export declare function getBundler(options: FarcasterContractOptions): ThirdwebContract;
//# sourceMappingURL=getBundler.d.ts.map