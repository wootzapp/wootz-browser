"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.signAuthorization = signAuthorization;
const parseAccount_js_1 = require("../../../accounts/utils/parseAccount.js");
const getChainId_js_1 = require("../../../actions/public/getChainId.js");
const getTransactionCount_js_1 = require("../../../actions/public/getTransactionCount.js");
const account_js_1 = require("../../../errors/account.js");
const getAction_js_1 = require("../../../utils/getAction.js");
async function signAuthorization(client, parameters) {
    const { account: account_ = client.account, contractAddress, chainId, nonce, } = parameters;
    if (!account_)
        throw new account_js_1.AccountNotFoundError({
            docsPath: '/experimental/eip7702/signAuthorization',
        });
    const account = (0, parseAccount_js_1.parseAccount)(account_);
    if (!account.experimental_signAuthorization)
        throw new account_js_1.AccountTypeNotSupportedError({
            docsPath: '/experimental/eip7702/signAuthorization',
            metaMessages: [
                'The `signAuthorization` Action does not support JSON-RPC Accounts.',
            ],
            type: account.type,
        });
    const authorization = {
        contractAddress,
        chainId,
        nonce,
    };
    if (typeof authorization.chainId === 'undefined')
        authorization.chainId =
            client.chain?.id ??
                (await (0, getAction_js_1.getAction)(client, getChainId_js_1.getChainId, 'getChainId')({}));
    if (typeof authorization.nonce === 'undefined') {
        authorization.nonce = await (0, getAction_js_1.getAction)(client, getTransactionCount_js_1.getTransactionCount, 'getTransactionCount')({
            address: account.address,
            blockTag: 'pending',
        });
    }
    return account.experimental_signAuthorization(authorization);
}
//# sourceMappingURL=signAuthorization.js.map