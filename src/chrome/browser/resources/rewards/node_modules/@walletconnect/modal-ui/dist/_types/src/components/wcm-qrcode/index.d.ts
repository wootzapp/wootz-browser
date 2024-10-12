import { LitElement } from 'lit';
export declare class WcmQrCode extends LitElement {
    static styles: any[];
    uri: string;
    size: number;
    imageId?: string;
    walletId?: string;
    imageUrl?: string;
    private svgTemplate;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-qrcode': WcmQrCode;
    }
}
