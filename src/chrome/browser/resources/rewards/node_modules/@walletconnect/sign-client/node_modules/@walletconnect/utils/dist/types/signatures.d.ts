import { AuthTypes } from "@walletconnect/types";
export declare function verifySignature(address: string, reconstructedMessage: string, cacaoSignature: AuthTypes.CacaoSignature, chainId: string, projectId: string, baseRpcUrl?: string): Promise<boolean>;
export declare function isValidEip191Signature(address: string, message: string, signature: string): boolean;
export declare function isValidEip1271Signature(address: string, reconstructedMessage: string, signature: string, chainId: string, projectId: string, baseRpcUrl?: string): Promise<boolean>;
//# sourceMappingURL=signatures.d.ts.map