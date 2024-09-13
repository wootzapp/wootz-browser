import { type BlockTag, type EIP1193RequestFn, type EIP1474Methods, type GetBlockReturnType } from "viem";
type GetBlockParameters<TIncludeTransactions extends boolean = false, TBlockTag extends BlockTag = "latest"> = {
    /** Whether or not to include transaction data in the response. */
    includeTransactions?: TIncludeTransactions;
} & ({
    /** The block number. */
    blockNumber?: bigint;
    blockTag?: never;
} | {
    blockNumber?: never;
    /**
     * The block tag.
     * default: 'latest'
     */
    blockTag?: TBlockTag | BlockTag;
});
/**
 * Retrieves a block by its number or tag from the Ethereum blockchain.
 * @param request - The EIP1193 request function.
 * @param params - The parameters for retrieving the block.
 * @returns A promise that resolves to the requested block.
 * @throws An error if the block is not found.
 * @rpc
 * @example
 * ```ts
 * import { getRpcClient, eth_getBlockByNumber } from "thirdweb/rpc";
 * const rpcRequest = getRpcClient({ client, chain });
 * const block = await eth_getBlockByNumber(rpcRequest, {
 *  blockNumber: 123456,
 *  includeTransactions: true,
 * });
 * ```
 */
export declare function eth_getBlockByNumber<TIncludeTransactions extends boolean = false, TBlockTag extends BlockTag = "latest">(request: EIP1193RequestFn<EIP1474Methods>, params: GetBlockParameters<TIncludeTransactions, TBlockTag>): Promise<GetBlockReturnType<undefined, TIncludeTransactions, TBlockTag>>;
export {};
//# sourceMappingURL=eth_getBlockByNumber.d.ts.map