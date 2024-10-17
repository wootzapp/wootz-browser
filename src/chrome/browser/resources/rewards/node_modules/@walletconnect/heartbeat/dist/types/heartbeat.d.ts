/// <reference types="node" />
import { EventEmitter } from "events";
import { IHeartBeat, HeartBeatOptions } from "./types";
export declare class HeartBeat extends IHeartBeat {
    static init(opts?: HeartBeatOptions): Promise<HeartBeat>;
    events: EventEmitter;
    interval: number;
    private intervalRef?;
    constructor(opts?: HeartBeatOptions);
    init(): Promise<void>;
    stop(): void;
    on(event: string, listener: any): void;
    once(event: string, listener: any): void;
    off(event: string, listener: any): void;
    removeListener(event: string, listener: any): void;
    private initialize;
    private pulse;
}
//# sourceMappingURL=heartbeat.d.ts.map