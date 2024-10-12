import { IWatch, TimestampInfo } from "./types";
export declare class Watch implements IWatch {
    timestamps: Map<string, TimestampInfo>;
    start(label: string): void;
    stop(label: string): void;
    get(label: string): TimestampInfo;
    elapsed(label: string): number;
}
export default Watch;
//# sourceMappingURL=watch.d.ts.map