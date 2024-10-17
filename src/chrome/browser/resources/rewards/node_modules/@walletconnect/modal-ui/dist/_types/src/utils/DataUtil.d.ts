export declare const DataUtil: {
    manualWallets(): {
        id: string;
        name: string;
        mobile: {
            native: string;
            universal?: string | undefined;
        };
        links: {
            native: string;
            universal?: string | undefined;
        };
    }[] | {
        id: string;
        name: string;
        desktop: {
            native: string;
            universal?: string | undefined;
        };
        links: {
            native: string;
            universal?: string | undefined;
        };
    }[];
    recentWallet(): import("@walletconnect/modal-core").WalletData | undefined;
    recomendedWallets(skipRecent?: boolean): import("@walletconnect/modal-core").Listing[];
};
