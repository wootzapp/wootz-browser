import 'stream';
import { _ as _WebSocket } from '../shared/crossws.a5db571c.mjs';
import 'events';
import 'http';
import 'crypto';
import 'https';
import 'net';
import 'tls';
import 'url';
import 'zlib';
import 'buffer';

const node = globalThis.WebSocket || _WebSocket;

export { node as default };
