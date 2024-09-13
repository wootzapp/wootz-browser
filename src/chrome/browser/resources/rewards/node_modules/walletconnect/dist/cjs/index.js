"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const tslib_1 = require("tslib");
const client_1 = tslib_1.__importDefault(require("@walletconnect/client"));
const qrcode_modal_1 = tslib_1.__importDefault(require("@walletconnect/qrcode-modal"));
const web3_provider_1 = tslib_1.__importDefault(require("@walletconnect/web3-provider"));
const utils_1 = require("@walletconnect/utils");
class WalletConnectSDK {
    constructor(options) {
        this.options = options;
    }
    get connected() {
        if (this.connector) {
            return this.connector.connected;
        }
        return false;
    }
    connect(createSessionOpts) {
        var _a;
        return tslib_1.__awaiter(this, void 0, void 0, function* () {
            const options = Object.assign({ bridge: "https://bridge.walletconnect.org", qrcodeModal: qrcode_modal_1.default }, this.options);
            if ((0, utils_1.isNode)()) {
                options.clientMeta = ((_a = this.options) === null || _a === void 0 ? void 0 : _a.clientMeta) || {
                    name: "WalletConnect SDK",
                    description: "WalletConnect SDK in NodeJS",
                    url: "#",
                    icons: ["https://walletconnect.org/walletconnect-logo.png"],
                };
            }
            const connector = new client_1.default(options);
            yield connector.connect(createSessionOpts);
            this.connector = connector;
            return connector;
        });
    }
    getWeb3Provider(opts) {
        if (!this.connector) {
            throw new Error("No connector available - please call connect() first");
        }
        return new web3_provider_1.default(Object.assign(Object.assign({}, opts), { connector: this.connector }));
    }
    getChannelProvider(opts) {
        if (!this.connector) {
            throw new Error("No connector available - please call connect() first");
        }
        throw new Error("Deprecated");
    }
    getStarkwareProvider(opts) {
        if (!this.connector) {
            throw new Error("No connector available - please call connect() first");
        }
        throw new Error("Deprecated");
    }
    getThreeIdProvider(opts) {
        if (!this.connector) {
            throw new Error("No connector available - please call connect() first");
        }
        throw new Error("Deprecated");
    }
}
exports.default = WalletConnectSDK;
//# sourceMappingURL=index.js.map