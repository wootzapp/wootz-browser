import "./strings.m.js";

import { loadTimeData } from "chrome://resources/js/load_time_data.js";
import { CrLitElement } from "//resources/lit/v3_0/lit.rollup.js";

import { getCss } from "./app.css.js";
import { getHtml } from "./app.html.js";

export class HelloWorldAppElement extends CrLitElement {
  static get is() {
    return "hello-world-app";
  }

  static override get styles() {
    return getCss();
  }

  override render() {
    return getHtml.bind(this)();
  }

  static override get properties() {
    return {
      message_: { type: String },
    };
  }

  protected message_: string = loadTimeData.getString("message");
}

declare global {
  interface HTMLElementTagNameMap {
    "hello-world-app": HelloWorldAppElement;
  }
}

customElements.define(HelloWorldAppElement.is, HelloWorldAppElement);
