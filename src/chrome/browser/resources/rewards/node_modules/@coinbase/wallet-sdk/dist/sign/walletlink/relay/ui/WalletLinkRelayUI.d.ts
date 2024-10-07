import { RelayUI } from './RelayUI';
export declare class WalletLinkRelayUI implements RelayUI {
    private readonly snackbar;
    private attached;
    constructor();
    attach(): void;
    showConnecting(options: {
        isUnlinkedErrorState?: boolean;
        onCancel: (error?: Error) => void;
        onResetConnection: () => void;
    }): () => void;
}
