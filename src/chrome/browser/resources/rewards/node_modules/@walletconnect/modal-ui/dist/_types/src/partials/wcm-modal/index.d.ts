import { LitElement } from 'lit';
export declare class WcmModal extends LitElement {
    static styles: any[];
    private open;
    private active;
    constructor();
    disconnectedCallback(): void;
    private readonly unsubscribeModal?;
    private abortController?;
    private get overlayEl();
    private get containerEl();
    private toggleBodyScroll;
    private onCloseModal;
    private onOpenModalEvent;
    private onCloseModalEvent;
    private addKeyboardEvents;
    private removeKeyboardEvents;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-modal': WcmModal;
    }
}
