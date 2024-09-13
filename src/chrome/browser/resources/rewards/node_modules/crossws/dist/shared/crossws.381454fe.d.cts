declare class WSError extends Error {
    constructor(...args: any[]);
}

declare class Message {
    readonly rawData: any;
    readonly isBinary?: boolean | undefined;
    constructor(rawData: any, isBinary?: boolean | undefined);
    text(): string;
    toString(): string;
}

type ReadyState = 0 | 1 | 2 | 3;
declare abstract class Peer<AdapterContext = any> implements WSRequest {
    ctx: AdapterContext;
    _subscriptions: Set<string>;
    static _idCounter: number;
    private _id;
    constructor(ctx: AdapterContext);
    get id(): string;
    get addr(): string | undefined;
    get url(): string;
    get headers(): HeadersInit;
    get readyState(): ReadyState | -1;
    abstract send(message: any, options?: {
        compress?: boolean;
    }): number;
    publish(topic: string, message: any, options?: {
        compress?: boolean;
    }): void;
    subscribe(topic: string): void;
    unsubscribe(topic: string): void;
    toString(): string;
}

type MaybePromise<T> = T | Promise<T>;
type Caller<T extends Record<string, (...args: any[]) => Promise<any>>, RT = null> = <K extends keyof T>(key: K, ...args: Parameters<T[K]>) => RT extends null ? Promise<ReturnType<T[K]>> : RT;
interface AdapterOptions {
    resolve?: ResolveHooks;
    hooks?: Hooks;
    adapterHooks?: AdapterHooks;
}
type Adapter<AdapterT, Options extends AdapterOptions> = (options?: Options) => AdapterT;
declare function defineWebSocketAdapter<AdapterT, Options extends AdapterOptions = AdapterOptions>(factory: Adapter<AdapterT, Options>): Adapter<AdapterT, Options>;
interface CrossWS {
    $callHook: Caller<AdapterHooks>;
    callHook: Caller<Exclude<Hooks, "upgrade">, void>;
    upgrade: (req: WSRequest) => Promise<{
        headers?: HeadersInit;
    }>;
}
interface WSRequest {
    readonly url: string;
    readonly headers: HeadersInit;
}
declare function defineHooks<T extends Partial<Hooks> = Partial<Hooks>>(hooks: T): T;
type ResolveHooks = (info: WSRequest | Peer) => Partial<Hooks> | Promise<Partial<Hooks>>;
type HookFn<ArgsT extends any[] = any, RT = void> = (info: Peer, ...args: ArgsT) => MaybePromise<RT>;
interface Hooks extends Record<string, HookFn<any[], any>> {
    /** Upgrading */
    upgrade: (req: WSRequest) => MaybePromise<void | {
        headers?: HeadersInit;
    }>;
    /** A message is received */
    message: (peer: Peer, message: Message) => MaybePromise<void>;
    /** A socket is opened */
    open: (peer: Peer) => MaybePromise<void>;
    /** A socket is closed */
    close: (peer: Peer, details: {
        code?: number;
        reason?: string;
    }) => MaybePromise<void>;
    /** An error occurs */
    error: (peer: Peer, error: WSError) => MaybePromise<void>;
}
interface AdapterHooks extends Record<string, HookFn<any[], any>> {
    "bun:message": HookFn<[ws: any, message: any]>;
    "bun:open": HookFn<[ws: any]>;
    "bun:close": HookFn<[ws: any]>;
    "bun:drain": HookFn<[]>;
    "bun:error": HookFn<[ws: any, error: any]>;
    "bun:ping": HookFn<[ws: any, data: any]>;
    "bun:pong": HookFn<[ws: any, data: any]>;
    "cloudflare:accept": HookFn<[]>;
    "cloudflare:message": HookFn<[event: any]>;
    "cloudflare:error": HookFn<[event: any]>;
    "cloudflare:close": HookFn<[event: any]>;
    "deno:open": HookFn<[]>;
    "deno:message": HookFn<[event: any]>;
    "deno:close": HookFn<[]>;
    "deno:error": HookFn<[error: any]>;
    "node:open": HookFn<[]>;
    "node:message": HookFn<[data: any, isBinary: boolean]>;
    "node:close": HookFn<[code: number, reason: Buffer]>;
    "node:error": HookFn<[error: any]>;
    "node:ping": HookFn<[data: Buffer]>;
    "node:pong": HookFn<[data: Buffer]>;
    "node:unexpected-response": HookFn<[req: any, res: any]>;
    "node:upgrade": HookFn<[req: any]>;
    "uws:open": HookFn<[ws: any]>;
    "uws:message": HookFn<[ws: any, message: any, isBinary: boolean]>;
    "uws:close": HookFn<[ws: any, code: number, message: any]>;
    "uws:ping": HookFn<[ws: any, message: any]>;
    "uws:pong": HookFn<[ws: any, message: any]>;
    "uws:drain": HookFn<[ws: any]>;
    "uws:upgrade": HookFn<[res: any, req: any, context: any]>;
    "uws:subscription": HookFn<[
        ws: any,
        topic: any,
        newCount: number,
        oldCount: number
    ]>;
}

export { type AdapterOptions as A, type CrossWS as C, type Hooks as H, Message as M, Peer as P, type ResolveHooks as R, WSError as W, type Caller as a, type Adapter as b, type WSRequest as c, defineWebSocketAdapter as d, defineHooks as e, type AdapterHooks as f };
