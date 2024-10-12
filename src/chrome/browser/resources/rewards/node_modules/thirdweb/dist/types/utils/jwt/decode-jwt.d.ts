import type { JWTPayload } from "./types.js";
/**
 * Decodes a JSON Web Token (JWT) and returns the decoded payload and signature.
 * @param jwt - The JWT string to decode.
 * @returns An object containing the decoded payload and signature.
 * @throws {Error} If the JWT header is invalid or if the JWT is invalid.
 * @example
 * ```ts
 * import { decodeJWT } from 'thirdweb/utils';
 *
 * const { payload, signature } = decodeJWT(jwt);
 * ```
 * @auth
 */
export declare function decodeJWT(jwt: string): {
    payload: JWTPayload;
    signature: `0x${string}`;
};
//# sourceMappingURL=decode-jwt.d.ts.map