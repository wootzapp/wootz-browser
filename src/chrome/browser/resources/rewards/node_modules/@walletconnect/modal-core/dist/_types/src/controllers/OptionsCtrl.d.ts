import type { OptionsCtrlState } from '../types/controllerTypes';
export declare const OptionsCtrl: {
    state: OptionsCtrlState;
    subscribe(callback: (newState: OptionsCtrlState) => void): () => void;
    setChains(chains: OptionsCtrlState['chains']): void;
    setWalletConnectUri(walletConnectUri: OptionsCtrlState['walletConnectUri']): void;
    setIsCustomDesktop(isCustomDesktop: OptionsCtrlState['isCustomDesktop']): void;
    setIsCustomMobile(isCustomMobile: OptionsCtrlState['isCustomMobile']): void;
    setIsDataLoaded(isDataLoaded: OptionsCtrlState['isDataLoaded']): void;
    setIsUiLoaded(isUiLoaded: OptionsCtrlState['isUiLoaded']): void;
    setIsAuth(isAuth: OptionsCtrlState['isAuth']): void;
};
