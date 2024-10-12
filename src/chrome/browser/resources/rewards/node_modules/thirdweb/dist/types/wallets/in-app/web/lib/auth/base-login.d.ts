import type { AuthAndWalletRpcReturnType, AuthLoginReturnType } from "../../../core/authentication/types.js";
import { AbstractLogin, type LoginQuerierTypes } from "./abstract-login.js";
/**
 *
 */
export declare class BaseLogin extends AbstractLogin<void, {
    email: string;
}, {
    email: string;
    otp: string;
    recoveryCode?: string;
}> {
    authenticateWithModal(): Promise<AuthAndWalletRpcReturnType>;
    /**
     * @internal
     */
    loginWithModal(): Promise<AuthLoginReturnType>;
    authenticateWithIframe({ email, }: {
        email: string;
    }): Promise<AuthAndWalletRpcReturnType>;
    /**
     * @internal
     */
    loginWithIframe({ email, }: {
        email: string;
    }): Promise<AuthLoginReturnType>;
    authenticateWithCustomJwt({ encryptionKey, jwt, }: LoginQuerierTypes["loginWithCustomJwt"]): Promise<AuthAndWalletRpcReturnType>;
    /**
     * @internal
     */
    loginWithCustomJwt({ encryptionKey, jwt, }: LoginQuerierTypes["loginWithCustomJwt"]): Promise<AuthLoginReturnType>;
    authenticateWithCustomAuthEndpoint({ encryptionKey, payload, }: LoginQuerierTypes["loginWithCustomAuthEndpoint"]): Promise<AuthAndWalletRpcReturnType>;
    /**
     * @internal
     */
    loginWithCustomAuthEndpoint({ encryptionKey, payload, }: LoginQuerierTypes["loginWithCustomAuthEndpoint"]): Promise<AuthLoginReturnType>;
    authenticateWithEmailOtp({ email, otp, recoveryCode, }: LoginQuerierTypes["verifyThirdwebEmailLoginOtp"]): Promise<AuthAndWalletRpcReturnType>;
    /**
     * @internal
     */
    loginWithEmailOtp({ email, otp, recoveryCode, }: LoginQuerierTypes["verifyThirdwebEmailLoginOtp"]): Promise<AuthLoginReturnType>;
    authenticateWithSmsOtp({ phoneNumber, otp, recoveryCode, }: LoginQuerierTypes["verifyThirdwebSmsLoginOtp"]): Promise<AuthAndWalletRpcReturnType>;
    /**
     * @internal
     */
    loginWithSmsOtp({ phoneNumber, otp, recoveryCode, }: LoginQuerierTypes["verifyThirdwebSmsLoginOtp"]): Promise<AuthLoginReturnType>;
}
//# sourceMappingURL=base-login.d.ts.map