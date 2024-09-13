import { SessionTypes } from "@walletconnect/types";
import { Namespace, NamespaceConfig } from "../types";
export declare function getRpcUrl(chainId: string, rpc: Namespace, projectId?: string): string | undefined;
export declare function getChainId(chain: string): string;
export declare function validateChainApproval(chain: string, chains: string[]): void;
export declare function getChainsFromApprovedSession(accounts: string[]): string[];
export declare function getAccountsFromSession(namespace: string, session: SessionTypes.Struct): string[];
export declare function mergeRequiredOptionalNamespaces(required?: NamespaceConfig, optional?: NamespaceConfig): NamespaceConfig;
export declare function normalizeNamespaces(namespaces: NamespaceConfig): NamespaceConfig;
export declare function parseCaip10Account(caip10Account: string): string;
export declare function populateNamespacesChains(namespaces: SessionTypes.Namespaces): Record<string, SessionTypes.Namespace>;
export declare function convertChainIdToNumber(chainId: string | number): number | string;
//# sourceMappingURL=misc.d.ts.map