import { LitElement } from 'lit';
export declare class WcmWalletConnectQr extends LitElement {
    static styles: any[];
    walletId?: string | undefined;
    imageId?: string | undefined;
    private uri?;
    constructor();
    private get overlayEl();
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-walletconnect-qr': WcmWalletConnectQr;
    }
}
