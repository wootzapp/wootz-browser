import type { TemplateResult } from 'lit';
import { LitElement } from 'lit';
export declare class WcmButton extends LitElement {
    static styles: any[];
    disabled?: boolean | undefined;
    iconLeft?: TemplateResult<2>;
    iconRight?: TemplateResult<2>;
    onClick: () => void;
    variant: 'default' | 'ghost' | 'outline';
    protected render(): TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-button': WcmButton;
    }
}
