import type { RouterCtrlState } from '../types/controllerTypes';
export declare const RouterCtrl: {
    state: RouterCtrlState;
    subscribe(callback: (newState: RouterCtrlState) => void): () => void;
    push(view: RouterCtrlState['view'], data?: RouterCtrlState['data']): void;
    reset(view: RouterCtrlState['view']): void;
    replace(view: RouterCtrlState['view']): void;
    goBack(): void;
    setData(data: RouterCtrlState['data']): void;
};
