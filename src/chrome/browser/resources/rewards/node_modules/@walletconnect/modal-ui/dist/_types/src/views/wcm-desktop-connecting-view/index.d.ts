import { LitElement } from 'lit';
export declare class WcmDesktopConnectingView extends LitElement {
    static styles: any[];
    isError: boolean;
    constructor();
    private onFormatAndRedirect;
    private openDesktopApp;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-desktop-connecting-view': WcmDesktopConnectingView;
    }
}
