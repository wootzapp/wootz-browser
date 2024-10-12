import type { Account } from "../../wallets/interfaces/wallet.js";
export type JWTPayloadInput<Tctx = unknown> = {
    iss: string;
    sub: string;
    aud: string;
    exp: Date;
    nbf: Date;
    iat: Date;
    jti?: string;
    ctx?: Tctx;
};
type EncodeJWTParams = {
    payload: JWTPayloadInput;
    account: Account;
};
/**
 * Builds a JSON Web Token (JWT) using the provided options.
 * @param options - The options for building the JWT.
 * @returns The generated JWT.
 * @throws Error if the account is not found.
 * @example
 * ```ts
 * import { encodeJWT } from 'thirdweb/utils';
 *
 * const jwt = await encodeJWT({
 *  payload: {
 *    iss: '0x1234567890123456789012345678901234567890',
 *    sub: '0x1234567890123456789012345678901234567890',
 *    aud: '0x1234567890123456789012345678901234567890',
 *    exp: new Date(Date.now() + 1000 * 60 * 60),
 *    nbf: new Date(),
 *    iat: new Date(),
 *    jti: '1234567890',
 *    ctx: {
 *        example: 'example',
 *    },
 *  },
 *  wallet,
 * });
 * ```
 * @auth
 */
export declare function encodeJWT(options: EncodeJWTParams): Promise<string>;
export {};
//# sourceMappingURL=encode-jwt.d.ts.map