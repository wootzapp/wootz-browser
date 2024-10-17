import { LitElement } from 'lit';
type Variant = 'primary' | 'secondary';
export declare class WcmButtonBig extends LitElement {
    static styles: any[];
    disabled?: boolean | undefined;
    variant?: Variant;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-button-big': WcmButtonBig;
    }
}
export {};
