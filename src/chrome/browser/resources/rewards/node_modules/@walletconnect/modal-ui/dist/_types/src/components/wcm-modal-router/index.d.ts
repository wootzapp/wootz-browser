import type { RouterView } from '@walletconnect/modal-core';
import { LitElement } from 'lit';
export declare class WcmModalRouter extends LitElement {
    static styles: any[];
    view: RouterView;
    prevView: RouterView;
    constructor();
    firstUpdated(): void;
    disconnectedCallback(): void;
    private readonly unsubscribe?;
    private oldHeight;
    private resizeObserver?;
    private get routerEl();
    private get contentEl();
    private viewTemplate;
    private onChangeRoute;
    protected render(): import("lit-html").TemplateResult<1>;
}
declare global {
    interface HTMLElementTagNameMap {
        'wcm-modal-router': WcmModalRouter;
    }
}
