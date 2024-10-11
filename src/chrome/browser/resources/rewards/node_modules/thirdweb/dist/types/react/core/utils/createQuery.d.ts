import { type UseQueryResult } from "@tanstack/react-query";
import type { Abi } from "abitype";
import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Creates a `useQuery` hook for a contract call.
 * @param readCall - A function that performs the contract function call and returns the result.
 * @returns An object containing the created `useRead` hook.
 * @example
 * ```jsx
 * import { createContractQuery } from "thirdweb/react";
 * import { totalSupply } from "thirdweb/extensions/erc20";
 * const useTotalSupply = createContractQuery(totalSupply);
 * const { data, isLoading } = useTotalSupply({contract})
 * ```
 * @transaction
 */
export declare function createContractQuery<opts extends object, result, abi extends Abi>(readCall: (options: BaseTransactionOptions<opts, abi>) => Promise<result>): (options: BaseTransactionOptions<opts, abi> & {
    queryOptions?: Partial<{
        enabled: boolean;
    }>;
}) => UseQueryResult<result, Error>;
//# sourceMappingURL=createQuery.d.ts.map