import type { Account } from "../../wallets/interfaces/wallet.js";
import type { Hex } from "../encoding/hex.js";
import type { Prettify } from "../type-utils.js";
type Message = Prettify<string | {
    raw: Hex | Uint8Array;
}>;
export type SignMessageOptions = {
    message: Message;
    privateKey: Hex;
};
/**
 * Signs a string message with a given private key.
 * @param options The options for signing.
 * @param options.message The message to be signed as a string or object containing raw hex or bytes
 * @param options.privateKey The private key to be used
 * @returns The signature as a hex string
 * @example
 * ```ts
 * import { signMessage } from "thirdweb/utils";
 * signMessage({
 *   message: "Hello, world!",
 *   privateKey: "0x...",
 * });
 * ```
 * @utils
 */
export declare function signMessage({ message, privateKey }: SignMessageOptions): Hex;
/**
 * Signs a string message with a given account.
 * @param options The options for signing.
 * @param options.message The message to be signed as a string or object containing raw hex or bytes
 * @param options.account The account to be used
 * @returns The signature as a hex string
 * @example
 * ```ts
 * import { signMessage } from "thirdweb/utils";
 * await signMessage({
 *   message: "Hello, world!",
 *   account
 * });
 * ```
 * @walletUtils
 */
export declare function signMessage({ message, account, }: {
    message: Message;
    account: Account;
}): Promise<Hex>;
export {};
//# sourceMappingURL=sign-message.d.ts.map