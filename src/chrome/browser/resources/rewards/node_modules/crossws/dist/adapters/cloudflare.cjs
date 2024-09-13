'use strict';

const types = require('../shared/crossws.36b9e66f.cjs');
const index = require('../index.cjs');

const cloudflare = types.defineWebSocketAdapter(
  (options = {}) => {
    const crossws = types.createCrossWS(options);
    const handleUpgrade = async (req, env, context) => {
      const pair = new WebSocketPair();
      const client = pair[0];
      const server = pair[1];
      const peer = new CloudflarePeer({
        cloudflare: { client, server, req, env, context }
      });
      const { headers } = await crossws.upgrade(peer);
      server.accept();
      crossws.$callHook("cloudflare:accept", peer);
      crossws.callHook("open", peer);
      server.addEventListener("message", (event) => {
        crossws.$callHook("cloudflare:message", peer, event);
        crossws.callHook("message", peer, new types.Message(event.data));
      });
      server.addEventListener("error", (event) => {
        crossws.$callHook("cloudflare:error", peer, event);
        crossws.callHook("error", peer, new index.WSError(event.error));
      });
      server.addEventListener("close", (event) => {
        crossws.$callHook("cloudflare:close", peer, event);
        crossws.callHook("close", peer, {
          code: event.code,
          reason: event.reason
        });
      });
      return new Response(null, {
        status: 101,
        webSocket: client,
        headers
      });
    };
    return {
      handleUpgrade
    };
  }
);
class CloudflarePeer extends types.Peer {
  get addr() {
    return void 0;
  }
  get url() {
    return this.ctx.cloudflare.req.url;
  }
  get headers() {
    return this.ctx.cloudflare.req.headers;
  }
  get readyState() {
    return this.ctx.cloudflare.client.readyState;
  }
  send(message) {
    this.ctx.cloudflare.server.send(types.toBufferLike(message));
    return 0;
  }
}

module.exports = cloudflare;
