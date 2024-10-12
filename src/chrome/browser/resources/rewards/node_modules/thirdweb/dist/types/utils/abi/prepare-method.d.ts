import { type AbiFunction, type ParseAbiItem } from "abitype";
import type { Hex } from "../encoding/hex.js";
type ParseMethod<method extends AbiFunction | `function ${string}`> = method extends AbiFunction ? method : method extends `function ${string}` ? ParseAbiItem<method> extends AbiFunction ? ParseAbiItem<method> : never : never;
export type PreparedMethod<TMethod extends AbiFunction | `function ${string}`> = [
    Hex,
    ParseMethod<TMethod>["inputs"],
    ParseMethod<TMethod>["outputs"]
];
/**
 * Prepares a method for usage by converting it into a prepared method object.
 * @param method The method to prepare.
 * @returns The prepared method object.
 * @example
 * ```ts
 * import { prepareMethod } from "thirdweb/utils";
 * const method = "function transfer(address to, uint256 value)";
 * const preparedMethod = prepareMethod(method);
 * ```
 * @contract
 */
export declare function prepareMethod<const TMethod extends AbiFunction | `function ${string}`>(method: TMethod): PreparedMethod<TMethod>;
export {};
//# sourceMappingURL=prepare-method.d.ts.map