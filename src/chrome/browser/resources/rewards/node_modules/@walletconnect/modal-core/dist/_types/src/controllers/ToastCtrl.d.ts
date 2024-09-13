import type { ToastCtrlState } from '../types/controllerTypes';
export declare const ToastCtrl: {
    state: ToastCtrlState;
    subscribe(callback: (newState: ToastCtrlState) => void): () => void;
    openToast(message: ToastCtrlState['message'], variant: ToastCtrlState['variant']): void;
    closeToast(): void;
};
