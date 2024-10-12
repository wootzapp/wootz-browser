import type { BaseTransactionOptions } from "../../../../../transaction/types.js";
import type { Hex } from "../../../../../utils/encoding/hex.js";
export declare const FN_SELECTOR: "0x89e04e0e";
/**
 * Checks if the `getModuleConfig` method is supported by the given contract.
 * @param availableSelectors An array of 4byte function selectors of the contract. You can get this in various ways, such as using "whatsabi" or if you have the ABI of the contract available you can use it to generate the selectors.
 * @returns A boolean indicating if the `getModuleConfig` method is supported.
 * @extension MODULES
 * @example
 * ```ts
 * import { isGetModuleConfigSupported } from "thirdweb/extensions/modules";
 * const supported = isGetModuleConfigSupported(["0x..."]);
 * ```
 */
export declare function isGetModuleConfigSupported(availableSelectors: string[]): boolean;
/**
 * Decodes the result of the getModuleConfig function call.
 * @param result - The hexadecimal result to decode.
 * @returns The decoded result as per the FN_OUTPUTS definition.
 * @extension MODULES
 * @example
 * ```ts
 * import { decodeGetModuleConfigResult } from "thirdweb/extensions/modules";
 * const result = decodeGetModuleConfigResultResult("...");
 * ```
 */
export declare function decodeGetModuleConfigResult(result: Hex): {
    registerInstallationCallback: boolean;
    requiredInterfaces: readonly `0x${string}`[];
    supportedInterfaces: readonly `0x${string}`[];
    callbackFunctions: readonly {
        selector: `0x${string}`;
    }[];
    fallbackFunctions: readonly {
        selector: `0x${string}`;
        permissionBits: bigint;
    }[];
};
/**
 * Calls the "getModuleConfig" function on the contract.
 * @param options - The options for the getModuleConfig function.
 * @returns The parsed result of the function call.
 * @extension MODULES
 * @example
 * ```ts
 * import { getModuleConfig } from "thirdweb/extensions/modules";
 *
 * const result = await getModuleConfig({
 *  contract,
 * });
 *
 * ```
 */
export declare function getModuleConfig(options: BaseTransactionOptions): Promise<{
    registerInstallationCallback: boolean;
    requiredInterfaces: readonly `0x${string}`[];
    supportedInterfaces: readonly `0x${string}`[];
    callbackFunctions: readonly {
        selector: `0x${string}`;
    }[];
    fallbackFunctions: readonly {
        selector: `0x${string}`;
        permissionBits: bigint;
    }[];
}>;
//# sourceMappingURL=getModuleConfig.d.ts.map