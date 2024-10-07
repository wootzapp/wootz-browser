export interface MobileWallet {
    id: string;
    name: string;
    links: {
        native: string;
        universal?: string;
    };
}
export interface DesktopWallet {
    id: string;
    name: string;
    links: {
        native: string;
        universal?: string;
    };
}
export interface ConfigCtrlState {
    projectId: string;
    chains?: string[];
    mobileWallets?: MobileWallet[];
    desktopWallets?: DesktopWallet[];
    walletImages?: Record<string, string>;
    enableAuthMode?: boolean;
    enableExplorer?: boolean;
    explorerRecommendedWalletIds?: string[] | 'NONE';
    explorerExcludedWalletIds?: string[] | 'ALL';
    termsOfServiceUrl?: string;
    privacyPolicyUrl?: string;
}
export interface ModalCtrlState {
    open: boolean;
}
export interface OptionsCtrlState {
    chains?: string[];
    walletConnectUri?: string;
    isAuth: boolean;
    isCustomDesktop: boolean;
    isCustomMobile: boolean;
    isDataLoaded: boolean;
    isUiLoaded: boolean;
}
export interface ExplorerCtrlState {
    wallets: ListingResponse & {
        page: number;
    };
    search: ListingResponse & {
        page: number;
    };
    recomendedWallets: Listing[];
}
export interface ListingParams {
    page?: number;
    search?: string;
    entries?: number;
    version?: number;
    chains?: string;
    recommendedIds?: string;
    excludedIds?: string;
    sdks?: string;
}
export interface Listing {
    id: string;
    name: string;
    homepage: string;
    image_id: string;
    app: {
        browser?: string;
        ios?: string;
        android?: string;
        mac?: string;
        windows?: string;
        linux?: string;
        chrome?: string;
        firefox?: string;
        safari?: string;
        edge?: string;
        opera?: string;
    };
    mobile: {
        native: string;
        universal: string;
    };
    desktop: {
        native: string;
        universal: string;
    };
}
export interface ListingResponse {
    listings: Listing[];
    total: number;
}
export interface ToastCtrlState {
    open: boolean;
    message: string;
    variant: 'error' | 'success';
}
export type RouterView = 'ConnectWallet' | 'DesktopConnecting' | 'InstallWallet' | 'MobileConnecting' | 'MobileQrcodeConnecting' | 'Qrcode' | 'WalletExplorer' | 'WebConnecting';
export interface WalletData {
    id: string;
    name: string;
    homepage?: string;
    image_id?: string;
    app?: {
        browser?: string;
        ios?: string;
        android?: string;
    };
    mobile?: {
        native?: string;
        universal?: string;
    };
    desktop?: {
        native?: string;
        universal?: string;
    };
}
export interface RouterCtrlState {
    history: RouterView[];
    view: RouterView;
    data?: {
        Wallet?: WalletData;
    };
}
export interface ThemeCtrlState {
    themeVariables?: {
        '--wcm-z-index'?: string;
        '--wcm-accent-color'?: string;
        '--wcm-accent-fill-color'?: string;
        '--wcm-background-color'?: string;
        '--wcm-background-border-radius'?: string;
        '--wcm-container-border-radius'?: string;
        '--wcm-wallet-icon-border-radius'?: string;
        '--wcm-wallet-icon-large-border-radius'?: string;
        '--wcm-wallet-icon-small-border-radius'?: string;
        '--wcm-input-border-radius'?: string;
        '--wcm-notification-border-radius'?: string;
        '--wcm-button-border-radius'?: string;
        '--wcm-secondary-button-border-radius'?: string;
        '--wcm-icon-button-border-radius'?: string;
        '--wcm-button-hover-highlight-border-radius'?: string;
        '--wcm-font-family'?: string;
        '--wcm-font-feature-settings'?: string;
        '--wcm-text-big-bold-size'?: string;
        '--wcm-text-big-bold-weight'?: string;
        '--wcm-text-big-bold-line-height'?: string;
        '--wcm-text-big-bold-letter-spacing'?: string;
        '--wcm-text-big-bold-text-transform'?: string;
        '--wcm-text-big-bold-font-family'?: string;
        '--wcm-text-medium-regular-size'?: string;
        '--wcm-text-medium-regular-weight'?: string;
        '--wcm-text-medium-regular-line-height'?: string;
        '--wcm-text-medium-regular-letter-spacing'?: string;
        '--wcm-text-medium-regular-text-transform'?: string;
        '--wcm-text-medium-regular-font-family'?: string;
        '--wcm-text-small-regular-size'?: string;
        '--wcm-text-small-regular-weight'?: string;
        '--wcm-text-small-regular-line-height'?: string;
        '--wcm-text-small-regular-letter-spacing'?: string;
        '--wcm-text-small-regular-text-transform'?: string;
        '--wcm-text-small-regular-font-family'?: string;
        '--wcm-text-small-thin-size'?: string;
        '--wcm-text-small-thin-weight'?: string;
        '--wcm-text-small-thin-line-height'?: string;
        '--wcm-text-small-thin-letter-spacing'?: string;
        '--wcm-text-small-thin-text-transform'?: string;
        '--wcm-text-small-thin-font-family'?: string;
        '--wcm-text-xsmall-bold-size'?: string;
        '--wcm-text-xsmall-bold-weight'?: string;
        '--wcm-text-xsmall-bold-line-height'?: string;
        '--wcm-text-xsmall-bold-letter-spacing'?: string;
        '--wcm-text-xsmall-bold-text-transform'?: string;
        '--wcm-text-xsmall-bold-font-family'?: string;
        '--wcm-text-xsmall-regular-size'?: string;
        '--wcm-text-xsmall-regular-weight'?: string;
        '--wcm-text-xsmall-regular-line-height'?: string;
        '--wcm-text-xsmall-regular-letter-spacing'?: string;
        '--wcm-text-xsmall-regular-text-transform'?: string;
        '--wcm-text-xsmall-regular-font-family'?: string;
        '--wcm-overlay-background-color'?: string;
        '--wcm-overlay-backdrop-filter'?: string;
    };
    themeMode?: 'dark' | 'light';
}
export type ModalEventData = {
    name: 'CONNECT_BUTTON';
} | {
    name: 'DISCONNECT_BUTTON';
} | {
    name: 'WALLET_BUTTON';
    walletId: string;
};
export interface ModalEvent {
    type: 'CLICK' | 'TRACK' | 'VIEW';
    name: ModalEventData['name'];
    timestamp: number;
    userSessionId: string;
    data?: ModalEventData;
}
export interface EventsCtrlState {
    enabled: boolean;
    userSessionId: string;
    events: ModalEvent[];
    connectedWalletId?: string;
}
