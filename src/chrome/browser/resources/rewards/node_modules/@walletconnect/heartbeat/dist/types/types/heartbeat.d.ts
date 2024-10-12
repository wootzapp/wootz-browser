import { IEvents } from "@walletconnect/events";
export interface HeartBeatOptions {
    interval?: number;
}
export declare abstract class IHeartBeat extends IEvents {
    abstract interval: number;
    constructor(opts?: HeartBeatOptions);
    abstract init(): Promise<void>;
    abstract stop(): void;
}
//# sourceMappingURL=heartbeat.d.ts.map