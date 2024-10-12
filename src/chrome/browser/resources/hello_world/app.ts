import './strings.m.js';


export class HelloWorldAppElement extends HTMLBaseElement {
  static get is() {
    return 'hello-world-app';
  }

  

  protected message_: string = "Hello Balram ";
}

declare global {
  interface HTMLElementTagNameMap {
    'hello-world-app': HelloWorldAppElement;
  }
}

customElements.define(HelloWorldAppElement.is, HelloWorldAppElement);