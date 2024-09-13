import { RandomSource } from "./";
export declare class BrowserRandomSource implements RandomSource {
    isAvailable: boolean;
    isInstantiated: boolean;
    private _crypto?;
    constructor();
    randomBytes(length: number): Uint8Array;
}
