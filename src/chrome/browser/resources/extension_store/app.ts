import './strings.m.js';


export class ExtensionStoreAppElement extends HTMLBaseElement {
  static get is() {
    return 'extension-store-app';
  }

  

  protected message_: string = "Hello Balram ";
}

declare global {
  interface HTMLElementTagNameMap {
    'extension-store-app': ExtensionStoreAppElement;
  }
}

customElements.define(ExtensionStoreAppElement.is, ExtensionStoreAppElement);