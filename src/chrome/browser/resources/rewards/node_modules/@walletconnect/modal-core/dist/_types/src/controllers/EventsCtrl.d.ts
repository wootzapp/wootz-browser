import type { EventsCtrlState, ModalEvent, ModalEventData } from '../types/controllerTypes';
export declare const EventsCtrl: {
    state: EventsCtrlState;
    subscribe(callback: (newEvent: ModalEvent) => void): () => void;
    initialize(): void;
    setConnectedWalletId(connectedWalletId: EventsCtrlState['connectedWalletId']): void;
    click(data: ModalEventData): void;
    track(data: ModalEventData): void;
    view(data: ModalEventData): void;
};
