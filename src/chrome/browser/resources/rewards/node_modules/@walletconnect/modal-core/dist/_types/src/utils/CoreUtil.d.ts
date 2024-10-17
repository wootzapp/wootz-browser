export declare const CoreUtil: {
    WALLETCONNECT_DEEPLINK_CHOICE: string;
    WCM_VERSION: string;
    RECOMMENDED_WALLET_AMOUNT: number;
    isMobile(): boolean;
    isAndroid(): boolean;
    isIos(): boolean;
    isHttpUrl(url: string): boolean;
    isArray<T>(data?: T | T[] | undefined): data is T[];
    formatNativeUrl(appUrl: string, wcUri: string, name: string): string;
    formatUniversalUrl(appUrl: string, wcUri: string, name: string): string;
    wait(miliseconds: number): Promise<unknown>;
    openHref(href: string, target: '_blank' | '_self'): void;
    setWalletConnectDeepLink(href: string, name: string): void;
    setWalletConnectAndroidDeepLink(wcUri: string): void;
    removeWalletConnectDeepLink(): void;
    setModalVersionInStorage(): void;
    getWalletRouterData(): import("../..").WalletData;
};
