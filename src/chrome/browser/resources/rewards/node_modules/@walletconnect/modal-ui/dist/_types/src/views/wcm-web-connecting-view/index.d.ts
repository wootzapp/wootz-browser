import { LitElement } from 'lit';
export declare class WcmWebConnectingView extends LitElement {
    static styles: any[];
    isError: boolean;
    constructor();
    private onFormatAndRedirect;
    private openWebWallet;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-web-connecting-view': WcmWebConnectingView;
    }
}
