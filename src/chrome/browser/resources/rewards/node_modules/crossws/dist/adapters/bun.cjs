'use strict';

const types = require('../shared/crossws.36b9e66f.cjs');

const bun = types.defineWebSocketAdapter(
  (options = {}) => {
    const crossws = types.createCrossWS(options);
    const getPeer = (ws) => {
      if (ws.data?._peer) {
        return ws.data._peer;
      }
      const peer = new BunPeer({ bun: { ws } });
      ws.data = ws.data || {};
      ws.data._peer = peer;
      return peer;
    };
    return {
      async handleUpgrade(req, server) {
        const { headers } = await crossws.upgrade({
          url: req.url,
          headers: req.headers
        });
        return server.upgrade(req, {
          data: { req, server },
          headers
        });
      },
      websocket: {
        message: (ws, message) => {
          const peer = getPeer(ws);
          crossws.$callHook("bun:message", peer, ws, message);
          crossws.callHook("message", peer, new types.Message(message));
        },
        open: (ws) => {
          const peer = getPeer(ws);
          crossws.$callHook("bun:open", peer, ws);
          crossws.callHook("open", peer);
        },
        close: (ws) => {
          const peer = getPeer(ws);
          crossws.$callHook("bun:close", peer, ws);
          crossws.callHook("close", peer, {});
        },
        drain: (ws) => {
          const peer = getPeer(ws);
          crossws.$callHook("bun:drain", peer);
        },
        ping(ws, data) {
          const peer = getPeer(ws);
          crossws.$callHook("bun:ping", peer, ws, data);
        },
        pong(ws, data) {
          const peer = getPeer(ws);
          crossws.$callHook("bun:pong", peer, ws, data);
        }
      }
    };
  }
);
class BunPeer extends types.Peer {
  get addr() {
    let addr = this.ctx.bun.ws.remoteAddress;
    if (addr.includes(":")) {
      addr = `[${addr}]`;
    }
    return addr;
  }
  get readyState() {
    return this.ctx.bun.ws.readyState;
  }
  get url() {
    return this.ctx.bun.ws.data.req?.url || "/";
  }
  get headers() {
    return this.ctx.bun.ws.data.req?.headers || new Headers();
  }
  send(message, options) {
    return this.ctx.bun.ws.send(types.toBufferLike(message), options?.compress);
  }
  publish(topic, message, options) {
    return this.ctx.bun.ws.publish(
      topic,
      types.toBufferLike(message),
      options?.compress
    );
  }
  subscribe(topic) {
    this.ctx.bun.ws.subscribe(topic);
  }
  unsubscribe(topic) {
    this.ctx.bun.ws.unsubscribe(topic);
  }
}

module.exports = bun;
