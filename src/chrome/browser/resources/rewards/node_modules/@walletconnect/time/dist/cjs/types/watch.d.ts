export interface TimestampInfo {
    started: number;
    elapsed?: number;
}
export declare abstract class IWatch {
    abstract timestamps: Map<string, TimestampInfo>;
    abstract start(label: string): void;
    abstract stop(label: string): void;
    abstract get(label: string): TimestampInfo;
    abstract elapsed(label: string): number;
}
//# sourceMappingURL=watch.d.ts.map