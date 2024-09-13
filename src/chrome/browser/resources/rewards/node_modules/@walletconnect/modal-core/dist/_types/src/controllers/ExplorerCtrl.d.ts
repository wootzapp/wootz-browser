import type { ExplorerCtrlState, ListingParams } from '../types/controllerTypes';
export declare const ExplorerCtrl: {
    state: ExplorerCtrlState;
    getRecomendedWallets(): Promise<import("../types/controllerTypes").Listing[]>;
    getWallets(params: ListingParams): Promise<(import("../types/controllerTypes").ListingResponse & {
        page: number;
    }) | {
        listings: import("../types/controllerTypes").Listing[];
        total: number;
    }>;
    getWalletImageUrl(imageId: string): string;
    getAssetImageUrl(imageId: string): string;
    resetSearch(): void;
};
