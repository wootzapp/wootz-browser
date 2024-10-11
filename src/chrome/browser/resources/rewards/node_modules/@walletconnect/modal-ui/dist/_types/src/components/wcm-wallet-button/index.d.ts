import { LitElement } from 'lit';
export declare class WcmWalletButton extends LitElement {
    static styles: any[];
    onClick: () => void;
    name: string;
    walletId: string;
    label?: string;
    imageId?: string;
    installed?: boolean | undefined;
    recent?: boolean | undefined;
    private sublabelTemplate;
    private handleClick;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-wallet-button': WcmWalletButton;
    }
}
