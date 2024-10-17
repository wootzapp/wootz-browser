import { LitElement } from 'lit';
export declare class WcmExplorerContext extends LitElement {
    private preload;
    constructor();
    private loadImages;
    private preloadListings;
    private preloadCustomImages;
    private preloadData;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-explorer-context': WcmExplorerContext;
    }
}
