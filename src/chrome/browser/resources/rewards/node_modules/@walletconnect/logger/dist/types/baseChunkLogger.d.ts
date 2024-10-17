import type { LoggerOptions } from "pino";
import LinkedList from "./linkedList";
export default class BaseChunkLogger {
    private logs;
    private level;
    private levelValue;
    private MAX_LOG_SIZE_IN_BYTES;
    constructor(level: LoggerOptions["level"], MAX_LOG_SIZE_IN_BYTES?: number);
    forwardToConsole(chunk: any, level: number): void;
    appendToLogs(chunk: any): void;
    getLogs(): LinkedList;
    clearLogs(): void;
    getLogArray(): string[];
    logsToBlob(extraMetadata: Record<string, string>): Blob;
}
//# sourceMappingURL=baseChunkLogger.d.ts.map