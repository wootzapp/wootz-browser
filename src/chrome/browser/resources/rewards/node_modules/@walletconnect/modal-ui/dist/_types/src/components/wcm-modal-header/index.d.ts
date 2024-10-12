import type { TemplateResult } from 'lit';
import { LitElement } from 'lit';
export declare class WcmModalHeader extends LitElement {
    static styles: any[];
    title: string;
    onAction?: () => void;
    actionIcon?: TemplateResult<2>;
    border: boolean;
    private backBtnTemplate;
    private actionBtnTemplate;
    protected render(): TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-modal-header': WcmModalHeader;
    }
}
