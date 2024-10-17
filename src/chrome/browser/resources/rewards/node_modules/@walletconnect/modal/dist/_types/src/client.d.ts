import type { ConfigCtrlState, ThemeCtrlState } from '@walletconnect/modal-core';
/**
 * Types
 */
export type WalletConnectModalConfig = ConfigCtrlState & ThemeCtrlState;
/**
 * Client
 */
export declare class WalletConnectModal {
    constructor(config: WalletConnectModalConfig);
    private initUi;
    openModal: (options?: import("packages/modal-core/dist/_types/src/controllers/ModalCtrl").OpenOptions | undefined) => Promise<void>;
    closeModal: () => void;
    subscribeModal: (callback: (newState: import("packages/modal-core/dist/_types/src/types/controllerTypes").ModalCtrlState) => void) => () => void;
    setTheme: (theme: ThemeCtrlState) => void;
}
