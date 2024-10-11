import { SessionTypes, ProposalTypes } from "@walletconnect/types";
interface ChainIdParams {
    namespace: string;
    reference: string;
}
interface AccountIdParams extends ChainIdParams {
    address: string;
}
export declare function parseChainId(chain: string): ChainIdParams;
export declare function formatChainId(params: ChainIdParams): string;
export declare function parseAccountId(account: string): AccountIdParams;
export declare function formatAccountId(params: AccountIdParams): string;
export declare function getUniqueValues(array: string[], parser: (str: string) => string): string[];
export declare function getAddressFromAccount(account: string): string;
export declare function getChainFromAccount(account: string): string;
export declare function formatAccountWithChain(address: string, chain: string): string;
export declare function getAddressesFromAccounts(accounts: string[]): string[];
export declare function getChainsFromAccounts(accounts: string[]): string[];
export declare function getAccountsFromNamespaces(namespaces: SessionTypes.Namespaces, keys?: string[]): string[];
export declare function getChainsFromNamespaces(namespaces: SessionTypes.Namespaces, keys?: string[]): string[];
export declare function getChainsFromRequiredNamespaces(requiredNamespaces: ProposalTypes.RequiredNamespaces, keys?: string[]): string[];
export declare function getChainsFromNamespace(namespace: string, namespaceProps: ProposalTypes.BaseRequiredNamespace): string[];
export {};
//# sourceMappingURL=caip.d.ts.map