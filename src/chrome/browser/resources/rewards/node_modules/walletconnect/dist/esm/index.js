import WalletConnect from "@walletconnect/client";
import QRCodeModal from "@walletconnect/qrcode-modal";
import Web3Provider from "@walletconnect/web3-provider";
import { isNode } from "@walletconnect/utils";
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
    async connect(createSessionOpts) {
        var _a;
        const options = Object.assign({ bridge: "https://bridge.walletconnect.org", qrcodeModal: QRCodeModal }, this.options);
        if (isNode()) {
            options.clientMeta = ((_a = this.options) === null || _a === void 0 ? void 0 : _a.clientMeta) || {
                name: "WalletConnect SDK",
                description: "WalletConnect SDK in NodeJS",
                url: "#",
                icons: ["https://walletconnect.org/walletconnect-logo.png"],
            };
        }
        const connector = new WalletConnect(options);
        await connector.connect(createSessionOpts);
        this.connector = connector;
        return connector;
    }
    getWeb3Provider(opts) {
        if (!this.connector) {
            throw new Error("No connector available - please call connect() first");
        }
        return new Web3Provider(Object.assign(Object.assign({}, opts), { connector: this.connector }));
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
export default WalletConnectSDK;
//# sourceMappingURL=index.js.map