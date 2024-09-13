import type { ListingParams, ListingResponse } from '../types/controllerTypes';
export declare const ExplorerUtil: {
    getDesktopListings(params: ListingParams): Promise<ListingResponse>;
    getMobileListings(params: ListingParams): Promise<ListingResponse>;
    getInjectedListings(params: ListingParams): Promise<ListingResponse>;
    getAllListings(params: ListingParams): Promise<ListingResponse>;
    getWalletImageUrl(imageId: string): string;
    getAssetImageUrl(imageId: string): string;
};
