import { d as defineWebSocketAdapter, c as createCrossWS, M as Message, P as Peer, t as toBufferLike } from '../shared/crossws.77e89680.mjs';

var __defProp = Object.defineProperty;
var __defNormalProp = (obj, key, value) => key in obj ? __defProp(obj, key, { enumerable: true, configurable: true, writable: true, value }) : obj[key] = value;
var __publicField = (obj, key, value) => {
  __defNormalProp(obj, typeof key !== "symbol" ? key + "" : key, value);
  return value;
};
const uws = defineWebSocketAdapter(
  (options = {}) => {
    const crossws = createCrossWS(options);
    const getPeer = (ws) => {
      const userData = ws.getUserData();
      if (userData._peer) {
        return userData._peer;
      }
      const peer = new UWSPeer({ uws: { ws, userData } });
      userData._peer = peer;
      return peer;
    };
    const websocket = {
      ...options.uws,
      close(ws, code, message) {
        const peer = getPeer(ws);
        crossws.$callHook("uws:close", peer, ws, code, message);
        crossws.callHook("close", peer, { code, reason: message?.toString() });
      },
      drain(ws) {
        const peer = getPeer(ws);
        crossws.$callHook("uws:drain", peer, ws);
      },
      message(ws, message, isBinary) {
        const peer = getPeer(ws);
        crossws.$callHook("uws:message", peer, ws, message, isBinary);
        const msg = new Message(message, isBinary);
        crossws.callHook("message", peer, msg);
      },
      open(ws) {
        const peer = getPeer(ws);
        crossws.$callHook("uws:open", peer, ws);
        crossws.callHook("open", peer);
      },
      ping(ws, message) {
        const peer = getPeer(ws);
        crossws.$callHook("uws:ping", peer, ws, message);
      },
      pong(ws, message) {
        const peer = getPeer(ws);
        crossws.$callHook("uws:pong", peer, ws, message);
      },
      subscription(ws, topic, newCount, oldCount) {
        const peer = getPeer(ws);
        crossws.$callHook(
          "uws:subscription",
          peer,
          ws,
          topic,
          newCount,
          oldCount
        );
      },
      async upgrade(res, req, context) {
        let aborted = false;
        res.onAborted(() => {
          aborted = true;
        });
        const { headers } = await crossws.upgrade({
          get url() {
            return req.getUrl();
          },
          get headers() {
            return _getHeaders(req);
          }
        });
        res.writeStatus("101 Switching Protocols");
        for (const [key, value] of new Headers(headers)) {
          res.writeHeader(key, value);
        }
        if (aborted) {
          return;
        }
        res.cork(() => {
          res.upgrade(
            {
              req,
              res,
              context
            },
            req.getHeader("sec-websocket-key"),
            req.getHeader("sec-websocket-protocol"),
            req.getHeader("sec-websocket-extensions"),
            context
          );
        });
      }
    };
    return {
      websocket
    };
  }
);
class UWSPeer extends Peer {
  constructor() {
    super(...arguments);
    __publicField(this, "_headers");
    __publicField(this, "_decoder", new TextDecoder());
  }
  get addr() {
    try {
      const addr = this._decoder.decode(
        this.ctx.uws.ws?.getRemoteAddressAsText()
      );
      return addr.replace(/(0000:)+/, "");
    } catch {
    }
  }
  // TODO
  // get readyState() {}
  get url() {
    return this.ctx.uws.userData.req.getUrl();
  }
  get headers() {
    if (!this._headers) {
      this._headers = _getHeaders(this.ctx.uws.userData.req);
    }
    return this._headers;
  }
  send(message, options) {
    return this.ctx.uws.ws.send(
      toBufferLike(message),
      options?.binary,
      options?.compress
    );
  }
  subscribe(topic) {
    this.ctx.uws.ws.subscribe(topic);
  }
  publish(topic, message, options) {
    this.ctx.uws.ws.publish(topic, message, options?.binary, options?.compress);
    return 0;
  }
}
function _getHeaders(req) {
  const headers = [];
  req.forEach((key, value) => {
    headers.push([key, value]);
  });
  return headers;
}

export { uws as default };
