import type { AuthOptions, LoginPayload } from "./types.js";
/**
 * @auth
 */
export type VerifyLoginPayloadParams = {
    payload: LoginPayload;
    signature: string;
};
declare const VERIFIED_SYMBOL: unique symbol;
/**
 * @auth
 */
export type VerifiedLoginPayload = LoginPayload & {
    [VERIFIED_SYMBOL]: true;
};
/**
 * @auth
 */
export type VerifyLoginPayloadResult = {
    valid: true;
    payload: VerifiedLoginPayload;
} | {
    valid: false;
    error: string;
};
/**
 * Verifies the login payload by checking various properties and signatures.
 * @param options - The authentication options.
 * @returns A function that accepts the login payload and signature, and performs the verification.
 * @internal
 */
export declare function verifyLoginPayload(options: AuthOptions): ({ payload, signature, }: VerifyLoginPayloadParams) => Promise<VerifyLoginPayloadResult>;
export {};
//# sourceMappingURL=verify-login-payload.d.ts.map