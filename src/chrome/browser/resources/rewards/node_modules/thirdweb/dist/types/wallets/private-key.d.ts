import type { ThirdwebClient } from "../client/client.js";
import type { Account } from "./interfaces/wallet.js";
export type PrivateKeyToAccountOptions = {
    /**
     * A client is the entry point to the thirdweb SDK.
     * It is required for all other actions.
     * You can create a client using the `createThirdwebClient` function. Refer to the [Creating a Client](https://portal.thirdweb.com/typescript/v5/client) documentation for more information.
     *
     * You must provide a `clientId` or `secretKey` in order to initialize a client. Pass `clientId` if you want for client-side usage and `secretKey` for server-side usage.
     *
     * ```tsx
     * import { createThirdwebClient } from "thirdweb";
     *
     * const client = createThirdwebClient({
     *  clientId: "<your_client_id>",
     * })
     * ```
     */
    client: ThirdwebClient;
    /**
     * The private key to use for the account.
     *
     * Do not commit private key in your code and use environment variables or other secure methods to store the private key.
     * @example
     * ```ts
     * const privateKey = process.env.PRIVATE_KEY;
     * ```
     */
    privateKey: string;
};
/**
 * Get an `Account` object from a private key.
 * @param options - The options for `privateKeyToAccount`
 * Refer to the type [`PrivateKeyToAccountOptions`](https://portal.thirdweb.com/references/typescript/v5/PrivateKeyToAccountOptions)
 * @returns The `Account` object that represents the private key
 * @example
 * ```ts
 * import { privateKeyToAccount } from "thirdweb/wallets"
 *
 * const wallet = privateKeyToAccount({
 *  client,
 *  privateKey: "...",
 * });
 * ```
 * @wallet
 */
export declare function privateKeyToAccount(options: PrivateKeyToAccountOptions): Account;
//# sourceMappingURL=private-key.d.ts.map