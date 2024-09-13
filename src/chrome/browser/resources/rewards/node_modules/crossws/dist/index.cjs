'use strict';

const types = require('./shared/crossws.36b9e66f.cjs');

class WSError extends Error {
  constructor(...args) {
    super(...args);
    this.name = "WSError";
  }
}

exports.Message = types.Message;
exports.Peer = types.Peer;
exports.createCrossWS = types.createCrossWS;
exports.defineHooks = types.defineHooks;
exports.defineWebSocketAdapter = types.defineWebSocketAdapter;
exports.WSError = WSError;
