import { LitElement } from 'lit';
type Variant = 'big-bold' | 'medium-regular' | 'small-regular' | 'small-thin' | 'xsmall-bold' | 'xsmall-regular';
type Color = 'accent' | 'error' | 'inverse' | 'primary' | 'secondary' | 'tertiary';
export declare class WcmText extends LitElement {
    static styles: any[];
    variant?: Variant;
    color?: Color | string;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-text': WcmText;
    }
}
export {};
