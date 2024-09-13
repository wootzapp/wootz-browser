import { prepareContractCall } from "../../../transaction/prepare-contract-call.js";
/**
 * Similar to the `release` extension, however this one requires you to specify a tokenAddress to release
 * @param options
 * @returns
 * @example
 * ```ts
 * import { releaseByToken } from "thirdweb/extensions/split";
 *
 * const transaction = releaseByToken({
 *  contract,
 *  account: ...,
 *  overrides: {
 *    ...
 *  }
 * });
 *
 * // Send the transaction
 * ...
 *
 * ```
 * @extension SPLIT
 */
export function releaseByToken(options) {
    return prepareContractCall({
        method: "function release(address token, address account)",
        params: [options.tokenAddress, options.account],
        ...options,
    });
}
//# sourceMappingURL=releaseByToken.js.map