import { A as AdapterOptions, b as Adapter } from '../shared/crossws.381454fe.js';
import * as _cf from '@cloudflare/workers-types';

type Env = Record<string, any>;
interface CloudflareAdapter {
    handleUpgrade(req: _cf.Request, env: Env, context: _cf.ExecutionContext): Promise<_cf.Response>;
}
interface CloudflareOptions extends AdapterOptions {
}
declare const _default: Adapter<CloudflareAdapter, CloudflareOptions>;

export { type CloudflareAdapter, type CloudflareOptions, _default as default };
