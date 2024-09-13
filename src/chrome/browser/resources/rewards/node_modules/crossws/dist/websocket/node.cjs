'use strict';

require('stream');
const websocket = require('../shared/crossws.2ed26345.cjs');
require('events');
require('http');
require('crypto');
require('https');
require('net');
require('tls');
require('url');
require('zlib');
require('buffer');

const node = globalThis.WebSocket || websocket._WebSocket;

module.exports = node;
