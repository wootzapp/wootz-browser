import type { DestinationStream, LoggerOptions } from "pino";
export default class ServerChunkLogger implements DestinationStream {
    private baseChunkLogger;
    constructor(level: LoggerOptions["level"], MAX_LOG_SIZE_IN_BYTES?: number);
    write(chunk: any): void;
    getLogs(): import("./linkedList").default;
    clearLogs(): void;
    getLogArray(): string[];
    logsToBlob(extraMetadata: Record<string, string>): Blob;
}
//# sourceMappingURL=serverChunkLogger.d.ts.map