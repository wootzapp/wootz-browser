import type { ThirdwebClient } from "../../client/client.js";
import type { Account } from "../interfaces/wallet.js";
export type GenerateAccountOptions = {
    client: ThirdwebClient;
};
/**
 * Generates a new account with a random private key.
 * @param options - The options for generating the account.
 * @param options.client - The Thirdweb client to use for the generated account.
 * @returns A Thirdweb account.
 * @example
 * ```ts
 * import { generateAccount } from "thirdweb/wallets";
 * const account = await generateAccount({ client });
 * ```
 * @walletUtils
 */
export declare function generateAccount(options: GenerateAccountOptions): Promise<Account>;
//# sourceMappingURL=generateAccount.d.ts.map