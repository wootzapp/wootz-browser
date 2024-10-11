import { LitElement } from 'lit';
export declare class WcmThemeContext extends LitElement {
    constructor();
    disconnectedCallback(): void;
    private readonly unsubscribeTheme?;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-theme-context': WcmThemeContext;
    }
}
