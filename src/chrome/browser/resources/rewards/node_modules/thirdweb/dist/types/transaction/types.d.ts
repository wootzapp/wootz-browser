import type { Abi, AbiFunction, AbiParameter, AbiParametersToPrimitiveTypes, ExtractAbiFunction, ParseAbiItem } from "abitype";
import type { TransactionReceipt as ViemTransactionReceipt } from "viem";
import type { ThirdwebContract } from "../contract/contract.js";
import type { Hex } from "../utils/encoding/hex.js";
import type { PreparedTransaction, StaticPrepareTransactionOptions } from "./prepare-transaction.js";
export type SendTransactionResult = {
    readonly transactionHash: Hex;
};
export type TransactionReceipt = ViemTransactionReceipt;
export type WithOverrides<T> = T & {
    overrides?: Omit<StaticPrepareTransactionOptions, "to" | "from" | "data" | "maxFeePerBlobGas" | "chain" | "client">;
};
export type ParamsOption<inputs extends readonly AbiParameter[]> = inputs extends {
    length: 0;
} ? {
    params?: readonly unknown[];
} : {
    params: Readonly<AbiParametersToPrimitiveTypes<inputs>> | (() => Promise<Readonly<AbiParametersToPrimitiveTypes<inputs>>>);
};
export type BaseTransactionOptions<T extends object = object, abi extends Abi = any> = {
    contract: ThirdwebContract<abi>;
} & T;
/**
 * Checks if the given value is of type TxOpts.
 * @param value - The value to check.
 * @returns True if the value is of type TxOpts, false otherwise.
 * @internal
 */
export declare function isBaseTransactionOptions(value: unknown): value is PreparedTransaction;
export type ParseMethod<abi extends Abi, method extends AbiFunction | string | ((contract: ThirdwebContract<abi>) => Promise<AbiFunction>)> = method extends AbiFunction ? method : method extends string ? method extends `function ${string}` ? ParseAbiItem<method> extends AbiFunction ? ParseAbiItem<method> : never : abi extends {
    length: 0;
} ? AbiFunction : ExtractAbiFunction<abi, method> : AbiFunction;
//# sourceMappingURL=types.d.ts.map