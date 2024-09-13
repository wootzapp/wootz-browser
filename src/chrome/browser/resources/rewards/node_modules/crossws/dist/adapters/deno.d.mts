import { A as AdapterOptions, b as Adapter } from '../shared/crossws.381454fe.mjs';
import * as _deno_types from '@deno/types';

interface DenoAdapter {
    handleUpgrade(req: Request, info: ServeHandlerInfo): Promise<Response>;
}
interface DenoOptions extends AdapterOptions {
}
declare global {
    const Deno: typeof _deno_types.Deno;
}
type ServeHandlerInfo = any;
declare const _default: Adapter<DenoAdapter, DenoOptions>;

export { type DenoAdapter, type DenoOptions, _default as default };
