import { LitElement } from 'lit';
export declare class WcmSearchInput extends LitElement {
    static styles: any[];
    onChange: () => null;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-search-input': WcmSearchInput;
    }
}
