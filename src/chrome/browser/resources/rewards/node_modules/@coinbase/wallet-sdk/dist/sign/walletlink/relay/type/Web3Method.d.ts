export declare const web3Methods: readonly ["requestEthereumAccounts", "signEthereumMessage", "signEthereumTransaction", "submitEthereumTransaction", "ethereumAddressFromSignedMessage", "scanQRCode", "generic", "childRequestEthereumAccounts", "addEthereumChain", "switchEthereumChain", "watchAsset", "selectProvider", "connectAndSignIn"];
export type Web3Method = (typeof web3Methods)[number];
