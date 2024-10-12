import { A as AdapterOptions, b as Adapter } from '../shared/crossws.381454fe.mjs';
import { WebSocketBehavior, HttpRequest, HttpResponse } from 'uWebSockets.js';

type UserData = {
    _peer?: any;
    req: HttpRequest;
    res: HttpResponse;
    context: any;
};
type WebSocketHandler = WebSocketBehavior<UserData>;
interface UWSAdapter {
    websocket: WebSocketHandler;
}
interface UWSOptions extends AdapterOptions {
    uws?: Exclude<WebSocketBehavior<any>, "close" | "drain" | "message" | "open" | "ping" | "pong" | "subscription" | "upgrade">;
}
declare const _default: Adapter<UWSAdapter, UWSOptions>;

export { type UWSAdapter, type UWSOptions, _default as default };
