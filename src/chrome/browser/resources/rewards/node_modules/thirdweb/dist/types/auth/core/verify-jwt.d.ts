import type { JWTPayload } from "../../utils/jwt/types.js";
import type { AuthOptions } from "./types.js";
/**
 * @auth
 */
type VerifyJWTParams = {
    jwt: string;
};
/**
 * @auth
 */
type VerifyJWTResult = {
    valid: true;
    parsedJWT: JWTPayload;
} | {
    valid: false;
    error: string;
};
/**
 * Verifies a JSON Web Token (JWT) based on the provided options.
 * @param options - The authentication options.
 * @returns A function that verifies the JWT based on the provided parameters.
 * @throws An error if no admin account is provided.
 * @internal
 */
export declare function verifyJWT(options: AuthOptions): (params: VerifyJWTParams) => Promise<VerifyJWTResult>;
export {};
//# sourceMappingURL=verify-jwt.d.ts.map