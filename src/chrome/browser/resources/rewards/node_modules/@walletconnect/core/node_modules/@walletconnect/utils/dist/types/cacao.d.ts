import { AuthTypes } from "@walletconnect/types";
export declare const getDidAddressSegments: (iss: string) => string[];
export declare const getDidChainId: (iss: string) => string | undefined;
export declare const getNamespacedDidChainId: (iss: string) => string | undefined;
export declare const getDidAddress: (iss: string) => string | undefined;
export declare function validateSignedCacao(params: {
    cacao: AuthTypes.Cacao;
    projectId?: string;
}): Promise<boolean>;
export declare const formatMessage: (cacao: AuthTypes.FormatMessageParams, iss: string) => string;
export declare function buildAuthObject(requestPayload: AuthTypes.PayloadParams, signature: AuthTypes.CacaoSignature, iss: string): AuthTypes.Cacao;
declare type PopulateAuthPayloadParams = {
    authPayload: AuthTypes.PayloadParams;
    chains: string[];
    methods: string[];
};
export declare function populateAuthPayload(params: PopulateAuthPayloadParams): AuthTypes.PayloadParams;
export declare function getDecodedRecapFromResources(resources?: string[]): RecapType | undefined;
export declare function recapHasResource(recap: any, resource: string): any;
export declare function getRecapResource(recap: any, resource: string): any[];
export declare function getRecapAbilitiesFromResource(actions: any[]): string[][];
export declare function getReCapActions(abilities: any[]): any[];
export declare function base64Encode(input: unknown): string;
export declare function base64Decode(encodedString: string): string;
export declare function isValidRecap(recap: any): void;
export declare function createRecap(resource: string, ability: string, actions: string[], limits?: {}): {
    att: {
        [x: string]: any;
    };
};
declare type RecapType = {
    att: {
        [key: string]: Record<string, unknown>;
    };
};
export declare function addResourceToRecap(recap: RecapType, resource: string, actions: unknown[]): {
    att: {};
};
export declare function assignAbilityToActions(ability: string, actions: string[], limits?: {}): any;
export declare function encodeRecap(recap: any): string;
export declare function decodeRecap(recap: any): RecapType;
export declare function createEncodedRecap(resource: string, ability: string, actions: string[]): string;
export declare function isRecap(resource: string): boolean | "";
export declare function mergeEncodedRecaps(recap1: string, recap2: string): string;
export declare function mergeRecaps(recap1: RecapType, recap2: RecapType): {
    att: {};
};
export declare function formatStatementFromRecap(statement: string | undefined, recap: RecapType): string;
export declare function getMethodsFromRecap(recap: string): any[];
export declare function getChainsFromRecap(recap: string): string[];
export declare function buildRecapStatement(statement: string, recap: unknown): string;
export declare function getRecapFromResources(resources?: string[]): string | undefined;
export {};
//# sourceMappingURL=cacao.d.ts.map