import type { WalletData } from '@walletconnect/modal-core';
import type { LitElement } from 'lit';
export declare const UiUtil: {
    MOBILE_BREAKPOINT: number;
    WCM_RECENT_WALLET_DATA: string;
    EXPLORER_WALLET_URL: string;
    getShadowRootElement(root: LitElement, selector: string): HTMLElement;
    getWalletIcon({ id, image_id }: {
        id: string;
        image_id?: string | undefined;
    }): string;
    getWalletName(name: string, short?: boolean): string;
    isMobileAnimation(): boolean;
    preloadImage(src: string): Promise<unknown>;
    getErrorMessage(err: unknown): string;
    debounce(func: (...args: any[]) => unknown, timeout?: number): (...args: unknown[]) => void;
    handleMobileLinking(wallet: WalletData): void;
    handleAndroidLinking(): void;
    handleUriCopy(): Promise<void>;
    getCustomImageUrls(): string[];
    truncate(value: string, strLen?: number): string;
    setRecentWallet(wallet: WalletData): void;
    getRecentWallet(): WalletData | undefined;
    caseSafeIncludes(str1: string, str2: string): boolean;
    openWalletExplorerUrl(): void;
    getCachedRouterWalletPlatforms(): {
        isDesktop: boolean;
        isMobile: boolean;
        isWeb: boolean;
    };
    goToConnectingView(wallet: WalletData): void;
};
