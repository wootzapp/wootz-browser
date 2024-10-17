import { RandomSource } from "./";
export declare class SystemRandomSource implements RandomSource {
    isAvailable: boolean;
    name: string;
    private _source;
    constructor();
    randomBytes(length: number): Uint8Array;
}
