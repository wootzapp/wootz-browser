import type { Abi, AbiFunction } from "abitype";
import type { Hex } from "../../utils/encoding/hex.js";
import type { PreparedTransaction } from "../prepare-transaction.js";
/**
 * Encodes a transaction object into a hexadecimal string representation of the encoded data.
 * @param transaction - The transaction object to encode.
 * @returns A promise that resolves to the encoded data as a hexadecimal string.
 * @transaction
 * @example
 * ```ts
 * import { encode } from "thirdweb";
 * const encodedData = await encode(transaction);
 * ```
 */
export declare function encode<abi extends Abi, abiFn extends AbiFunction>(transaction: PreparedTransaction<abi, abiFn>): Promise<Hex>;
/**
 * Get the transaction.data (from a PreparedTransaction)
 * If the transaction does not have `data`, we default to "0x"
 * @internal
 */
export declare function getDataFromTx<abi extends Abi, abiFn extends AbiFunction>(transaction: PreparedTransaction<abi, abiFn>): Promise<Hex>;
/**
 * Get the extraCallData from a PreparedTransaction
 * @internal
 * If extraCallData is "0x", we will return `undefined`
 * to simplify the code, since concatenating "0x" doesn't do anything
 */
export declare function getExtraCallDataFromTx<abi extends Abi, abiFn extends AbiFunction>(transaction: PreparedTransaction<abi, abiFn>): Promise<Hex | undefined>;
//# sourceMappingURL=encode.d.ts.map