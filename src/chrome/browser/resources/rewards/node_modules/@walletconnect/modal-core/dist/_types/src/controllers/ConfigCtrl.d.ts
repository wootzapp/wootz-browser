import type { ConfigCtrlState } from '../types/controllerTypes';
export declare const ConfigCtrl: {
    state: ConfigCtrlState;
    subscribe(callback: (newState: ConfigCtrlState) => void): () => void;
    setConfig(config: ConfigCtrlState): void;
};
