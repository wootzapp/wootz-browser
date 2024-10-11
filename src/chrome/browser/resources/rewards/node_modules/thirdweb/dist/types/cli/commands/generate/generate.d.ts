export declare function generate(input: ChainIdAndContract): Promise<void>;
type ChainIdAndContract = `${number}/0x${string}`;
export declare function isValidChainIdAndContractAddress(chainIdPlusContract: unknown): chainIdPlusContract is ChainIdAndContract;
export {};
//# sourceMappingURL=generate.d.ts.map