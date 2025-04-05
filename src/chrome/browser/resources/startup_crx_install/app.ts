import './strings.m.js';


export class StartupCrxInstallAppElement extends HTMLBaseElement {
  static get is() {
    return 'startup-crx-install-app';
  }

  

  protected message_: string = "Hello Startup Crx Install ";
}

declare global {
  interface HTMLElementTagNameMap {
    'startup-crx-install-app': StartupCrxInstallAppElement;
  }
}

customElements.define(StartupCrxInstallAppElement.is, StartupCrxInstallAppElement);