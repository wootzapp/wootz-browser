export { M as Message, P as Peer, c as createCrossWS, a as defineHooks, d as defineWebSocketAdapter } from './shared/crossws.77e89680.mjs';

class WSError extends Error {
  constructor(...args) {
    super(...args);
    this.name = "WSError";
  }
}

export { WSError };
