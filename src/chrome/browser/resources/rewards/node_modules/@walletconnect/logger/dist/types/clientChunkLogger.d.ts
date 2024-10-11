import type { LoggerOptions } from "pino";
export default class ClientChunkLogger {
    private baseChunkLogger;
    constructor(level: LoggerOptions["level"], MAX_LOG_SIZE_IN_BYTES?: number);
    write(chunk: any): void;
    getLogs(): import("./linkedList").default;
    clearLogs(): void;
    getLogArray(): string[];
    logsToBlob(extraMetadata: Record<string, string>): Blob;
    downloadLogsBlobInBrowser(extraMetadata: Record<string, string>): void;
}
//# sourceMappingURL=clientChunkLogger.d.ts.map