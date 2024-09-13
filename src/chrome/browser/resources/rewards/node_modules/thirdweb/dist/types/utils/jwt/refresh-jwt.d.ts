import type { Account } from "../../wallets/interfaces/wallet.js";
export type RefreshJWTParams = {
    account: Account;
    jwt: string;
    expirationTime?: number;
};
/**
 * Refreshes a JSON Web Token (JWT) by encoding a new payload with updated expiration time.
 * @param options - The options for refreshing the JWT.
 * @returns A Promise that resolves to the refreshed JWT.
 * @example
 * ```ts
 * import { refreshJWT } from 'thirdweb/utils';
 *
 * const jwt = await refreshJWT({
 *  account,
 *  jwt,
 *  expirationTime: 1000 * 60 * 60,
 * });
 * ```
 * @auth
 */
export declare function refreshJWT(options: RefreshJWTParams): Promise<string>;
//# sourceMappingURL=refresh-jwt.d.ts.map