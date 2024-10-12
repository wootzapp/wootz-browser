function createCrossWS(opts = {}) {
  const resolveHook = async (req, name) => {
    const hooks = await opts.resolve?.(req);
    return hooks?.[name];
  };
  return {
    // WS Hooks
    async callHook(name, ...args) {
      await opts.hooks?.[name]?.apply(void 0, args);
      const hook = await resolveHook(args[0], name);
      await hook?.apply(void 0, args);
    },
    // Upgrade
    async upgrade(req) {
      const [res1, res2] = await Promise.all([
        opts.hooks?.upgrade?.(req),
        await resolveHook(req, "upgrade").then((h) => h?.(req))
      ]);
      const headers = new Headers(res1?.headers);
      if (res2?.headers) {
        for (const [key, value] of new Headers(res2?.headers)) {
          headers.append(key, value);
        }
      }
      return { headers };
    },
    // Adapter hook
    $callHook(name, ...args) {
      return opts.adapterHooks?.[name]?.apply(void 0, args);
    }
  };
}

function toBufferLike(val) {
  if (val === void 0 || val === null) {
    return "";
  }
  if (typeof val === "string") {
    return val;
  }
  if (isPlainObject(val)) {
    return JSON.stringify(val);
  }
  return val;
}
function isPlainObject(value) {
  if (value === null || typeof value !== "object") {
    return false;
  }
  const prototype = Object.getPrototypeOf(value);
  if (prototype !== null && prototype !== Object.prototype && Object.getPrototypeOf(prototype) !== null) {
    return false;
  }
  if (Symbol.iterator in value) {
    return false;
  }
  if (Symbol.toStringTag in value) {
    return Object.prototype.toString.call(value) === "[object Module]";
  }
  return true;
}

class Message {
  constructor(rawData, isBinary) {
    this.rawData = rawData;
    this.isBinary = isBinary;
  }
  text() {
    if (typeof this.rawData === "string") {
      return this.rawData;
    }
    const buff = toBufferLike(this.rawData);
    if (typeof buff === "string") {
      return buff;
    }
    return new TextDecoder().decode(buff);
  }
  toString() {
    return this.text();
  }
  [Symbol.for("nodejs.util.inspect.custom")]() {
    return this.text();
  }
}

var __defProp = Object.defineProperty;
var __defNormalProp = (obj, key, value) => key in obj ? __defProp(obj, key, { enumerable: true, configurable: true, writable: true, value }) : obj[key] = value;
var __publicField = (obj, key, value) => {
  __defNormalProp(obj, typeof key !== "symbol" ? key + "" : key, value);
  return value;
};
const ReadyStateMap = {
  "-1": "unknown",
  0: "connecting",
  1: "open",
  2: "closing",
  3: "closed"
};
const _Peer = class _Peer {
  constructor(ctx) {
    this.ctx = ctx;
    __publicField(this, "_subscriptions", /* @__PURE__ */ new Set());
    __publicField(this, "_id");
    this._id = ++_Peer._idCounter + "";
  }
  get id() {
    return this._id.toString();
  }
  get addr() {
    return void 0;
  }
  get url() {
    return "/";
  }
  get headers() {
    return {};
  }
  get readyState() {
    return -1;
  }
  publish(topic, message, options) {
  }
  subscribe(topic) {
    this._subscriptions.add(topic);
  }
  unsubscribe(topic) {
    this._subscriptions.delete(topic);
  }
  toString() {
    return `#${this.id}`;
  }
  [Symbol.for("nodejs.util.inspect.custom")]() {
    const _id = this.toString();
    const _addr = this.addr ? ` (${this.addr})` : "";
    const _state = this.readyState === 1 || this.readyState === -1 ? "" : ` [${ReadyStateMap[this.readyState]}]`;
    return `${_id}${_addr}${_state}`;
  }
};
__publicField(_Peer, "_idCounter", 0);
let Peer = _Peer;

function defineWebSocketAdapter(factory) {
  return factory;
}
function defineHooks(hooks) {
  return hooks;
}

export { Message as M, Peer as P, defineHooks as a, createCrossWS as c, defineWebSocketAdapter as d, toBufferLike as t };
