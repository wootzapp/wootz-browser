import { LitElement } from 'lit';
export declare class WcmWalletImage extends LitElement {
    static styles: any[];
    walletId: string;
    imageId?: string;
    imageUrl?: string;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-wallet-image': WcmWalletImage;
    }
}
