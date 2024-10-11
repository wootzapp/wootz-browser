import type { ThemeCtrlState } from '../types/controllerTypes';
export declare const ThemeCtrl: {
    state: ThemeCtrlState;
    subscribe(callback: (newState: ThemeCtrlState) => void): () => void;
    setThemeConfig(theme: ThemeCtrlState): void;
};
