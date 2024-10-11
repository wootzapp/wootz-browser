'use strict';

const types = require('../shared/crossws.36b9e66f.cjs');
const index = require('../index.cjs');

const deno = types.defineWebSocketAdapter(
  (options = {}) => {
    const crossws = types.createCrossWS(options);
    const handleUpgrade = async (req, info) => {
      const { headers } = await crossws.upgrade({
        url: req.url,
        headers: req.headers
      });
      const upgrade = Deno.upgradeWebSocket(req, {
        // @ts-expect-error https://github.com/denoland/deno/pull/22242
        headers
      });
      const peer = new DenoPeer({
        deno: { ws: upgrade.socket, req, info }
      });
      upgrade.socket.addEventListener("open", () => {
        crossws.$callHook("deno:open", peer);
        crossws.callHook("open", peer);
      });
      upgrade.socket.addEventListener("message", (event) => {
        crossws.$callHook("deno:message", peer, event);
        crossws.callHook("message", peer, new types.Message(event.data));
      });
      upgrade.socket.addEventListener("close", () => {
        crossws.$callHook("deno:close", peer);
        crossws.callHook("close", peer, {});
      });
      upgrade.socket.addEventListener("error", (error) => {
        crossws.$callHook("deno:error", peer, error);
        crossws.callHook("error", peer, new index.WSError(error));
      });
      return upgrade.response;
    };
    return {
      handleUpgrade
    };
  }
);
class DenoPeer extends types.Peer {
  get addr() {
    return this.ctx.deno.ws.remoteAddress;
  }
  get readyState() {
    return this.ctx.deno.ws.readyState;
  }
  get url() {
    return this.ctx.deno.req.url;
  }
  get headers() {
    return this.ctx.deno.req.headers || new Headers();
  }
  send(message) {
    this.ctx.deno.ws.send(types.toBufferLike(message));
    return 0;
  }
}

module.exports = deno;
