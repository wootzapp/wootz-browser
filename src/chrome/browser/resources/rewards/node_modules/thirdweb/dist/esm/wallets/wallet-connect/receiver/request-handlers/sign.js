import { validateAccountAddress } from "../utils.js";
/**
 * @internal
 */
export async function handleSignRequest(options) {
    const { account, params } = options;
    validateAccountAddress(account, params[1]);
    return account.signMessage({ message: { raw: params[0] } });
}
//# sourceMappingURL=sign.js.map