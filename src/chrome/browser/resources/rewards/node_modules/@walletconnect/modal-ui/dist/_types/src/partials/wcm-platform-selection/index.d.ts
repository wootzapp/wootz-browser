import { LitElement } from 'lit';
export declare class WcmPlatformSelection extends LitElement {
    static styles: any[];
    isMobile: boolean;
    isDesktop: boolean;
    isWeb: boolean;
    isRetry: boolean;
    private onMobile;
    private onDesktop;
    private onWeb;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-platform-selection': WcmPlatformSelection;
    }
}
