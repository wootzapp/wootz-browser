/// <reference types="node" />
import { EventEmitter } from "events";
import { CoreTypes, ICore } from "@walletconnect/types";
export declare class Core extends ICore {
    readonly protocol = "wc";
    readonly version = 2;
    readonly name: ICore["name"];
    readonly relayUrl: ICore["relayUrl"];
    readonly projectId: ICore["projectId"];
    readonly customStoragePrefix: ICore["customStoragePrefix"];
    events: ICore["events"];
    logger: ICore["logger"];
    heartbeat: ICore["heartbeat"];
    relayer: ICore["relayer"];
    crypto: ICore["crypto"];
    storage: ICore["storage"];
    history: ICore["history"];
    expirer: ICore["expirer"];
    pairing: ICore["pairing"];
    verify: ICore["verify"];
    echoClient: ICore["echoClient"];
    linkModeSupportedApps: ICore["linkModeSupportedApps"];
    eventClient: ICore["eventClient"];
    private initialized;
    private logChunkController;
    static init(opts?: CoreTypes.Options): Promise<Core>;
    constructor(opts?: CoreTypes.Options);
    get context(): string;
    start(): Promise<void>;
    getLogsBlob(): Promise<Blob | undefined>;
    addLinkModeSupportedApp(universalLink: string): Promise<void>;
    on: (name: any, listener: any) => EventEmitter;
    once: (name: any, listener: any) => EventEmitter;
    off: (name: any, listener: any) => EventEmitter;
    removeListener: (name: any, listener: any) => EventEmitter;
    dispatchEnvelope: ({ topic, message, sessionExists, }: {
        topic: string;
        message: string;
        sessionExists: boolean;
    }) => void;
    private initialize;
}
//# sourceMappingURL=core.d.ts.map