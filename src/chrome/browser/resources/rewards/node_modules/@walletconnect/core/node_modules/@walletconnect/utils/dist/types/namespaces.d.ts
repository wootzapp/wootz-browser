import { ProposalTypes, SessionTypes } from "@walletconnect/types";
export declare function getAccountsChains(accounts: SessionTypes.Namespace["accounts"]): string[];
export declare function getNamespacesChains(namespaces: SessionTypes.Namespaces): string[];
export declare function getNamespacesMethodsForChainId(namespaces: SessionTypes.Namespaces, chainId: string): string[];
export declare function getNamespacesEventsForChainId(namespaces: SessionTypes.Namespaces, chainId: string): string[];
export declare function getRequiredNamespacesFromNamespaces(namespaces: SessionTypes.Namespaces, caller: string): ProposalTypes.RequiredNamespaces;
export declare type BuildApprovedNamespacesParams = {
    proposal: ProposalTypes.Struct;
    supportedNamespaces: Record<string, {
        chains: string[];
        methods: string[];
        events: string[];
        accounts: string[];
    }>;
};
export declare function buildApprovedNamespaces(params: BuildApprovedNamespacesParams): SessionTypes.Namespaces;
export declare function isCaipNamespace(namespace: string): boolean;
export declare function parseNamespaceKey(namespace: string): string;
export declare function normalizeNamespaces(namespaces: ProposalTypes.RequiredNamespaces): ProposalTypes.RequiredNamespaces;
export declare function getNamespacesFromAccounts(accounts: string[]): {};
export declare function buildNamespacesFromAuth(methods: string[], accounts: string[]): {};
//# sourceMappingURL=namespaces.d.ts.map