import { A as AdapterOptions, P as Peer, b as Adapter } from '../shared/crossws.381454fe.cjs';
import { WebSocketHandler, Server } from 'bun';

interface BunAdapter {
    websocket: WebSocketHandler<ContextData>;
    handleUpgrade(req: Request, server: Server): Promise<boolean>;
}
interface BunOptions extends AdapterOptions {
}
type ContextData = {
    _peer?: Peer;
    req?: Request;
    server?: Server;
};
declare const _default: Adapter<BunAdapter, BunOptions>;

export { type BunAdapter, type BunOptions, _default as default };
