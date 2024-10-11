import { LitElement } from 'lit';
export declare class WcmWalletExplorerView extends LitElement {
    static styles: any[];
    private loading;
    private firstFetch;
    private search;
    private endReached;
    firstUpdated(): void;
    disconnectedCallback(): void;
    private get placeholderEl();
    private intersectionObserver;
    private createPaginationObserver;
    private isLastPage;
    private fetchWallets;
    private onConnect;
    private onSearchChange;
    private readonly searchDebounce;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-wallet-explorer-view': WcmWalletExplorerView;
    }
}
