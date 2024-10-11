import { LitElement } from 'lit';
export declare class WcmMobileConnectingView extends LitElement {
    static styles: any[];
    isError: boolean;
    constructor();
    private onFormatAndRedirect;
    private openMobileApp;
    private onGoToAppStore;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-mobile-connecting-view': WcmMobileConnectingView;
    }
}
