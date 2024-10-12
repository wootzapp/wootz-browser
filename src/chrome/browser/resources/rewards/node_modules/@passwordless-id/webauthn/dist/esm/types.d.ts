export type AuthType = 'auto' | 'local' | 'extern' | 'roaming' | 'both';
export type NumAlgo = -7 | -257;
export type NamedAlgo = 'RS256' | 'ES256';
export interface CommonOptions {
    domain?: string;
    userVerification?: UserVerificationRequirement;
    authenticatorType?: AuthType;
    timeout?: number;
    debug?: boolean;
    signal?: AbortSignal;
}
export interface AuthenticateOptions extends CommonOptions {
    mediation?: CredentialMediationRequirement;
}
export interface AuthenticationEncoded {
    credentialId: string;
    authenticatorData: string;
    clientData: string;
    signature: string;
    userHandle?: string;
}
export interface AuthenticationParsed {
    credentialId: string;
    authenticator: AuthenticatorInfo;
    client: ClientInfo;
    signature: string;
}
export interface RegisterOptions extends CommonOptions {
    userHandle?: string;
    attestation?: boolean;
    discoverable?: ResidentKeyRequirement;
}
export interface CredentialKey {
    id: string;
    publicKey: string;
    algorithm: 'RS256' | 'ES256';
}
export interface RegistrationEncoded {
    username: string;
    credential: CredentialKey;
    authenticatorData: string;
    clientData: string;
    attestationData?: string;
}
export interface RegistrationParsed {
    username: string;
    credential: {
        id: string;
        publicKey: string;
        algorithm: 'RS256' | 'ES256';
    };
    authenticator: AuthenticatorInfo;
    client: ClientInfo;
    attestation?: any;
}
export interface ClientInfo {
    type: "webauthn.create" | "webauthn.get";
    challenge: string;
    origin: string;
    crossOrigin: boolean;
    tokenBindingId?: {
        id: string;
        status: string;
    };
    extensions?: any;
}
export interface AuthenticatorInfo {
    rpIdHash: string;
    flags: {
        userPresent: boolean;
        userVerified: boolean;
        backupEligibility: boolean;
        backupState: boolean;
        attestedData: boolean;
        extensionsIncluded: boolean;
    };
    counter: number;
    aaguid: string;
    name: string;
    icon_light: string;
    icon_dark: string;
}
