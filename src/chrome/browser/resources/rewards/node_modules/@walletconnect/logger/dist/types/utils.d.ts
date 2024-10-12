import { Logger, LoggerOptions } from "pino";
import ClientChunkLogger from "./clientChunkLogger";
import ServerChunkLogger from "./serverChunkLogger";
import BaseChunkLogger from "./baseChunkLogger";
export interface ChunkLoggerController {
    logsToBlob: BaseChunkLogger["logsToBlob"];
    getLogArray: () => string[];
    clearLogs: () => void;
    downloadLogsBlobInBrowser?: ClientChunkLogger["downloadLogsBlobInBrowser"];
}
export declare function getDefaultLoggerOptions(opts?: LoggerOptions): LoggerOptions;
export declare function getBrowserLoggerContext(logger: Logger, customContextKey?: string): string;
export declare function setBrowserLoggerContext(logger: Logger, context: string, customContextKey?: string): Logger;
export declare function getLoggerContext(logger: Logger, customContextKey?: string): string;
export declare function formatChildLoggerContext(logger: Logger, childContext: string, customContextKey?: string): string;
export declare function generateChildLogger(logger: Logger, childContext: string, customContextKey?: string): Logger;
export declare function generateClientLogger(params: {
    opts?: LoggerOptions;
    maxSizeInBytes?: number;
}): {
    logger: Logger<any>;
    chunkLoggerController: ClientChunkLogger;
};
export declare function generateServerLogger(params: {
    maxSizeInBytes?: number;
    opts?: LoggerOptions;
}): {
    logger: Logger<any>;
    chunkLoggerController: ServerChunkLogger;
};
export declare function generatePlatformLogger(params: {
    maxSizeInBytes?: number;
    opts?: LoggerOptions;
    loggerOverride?: string | Logger<any>;
}): {
    logger: Logger<any>;
    chunkLoggerController: ChunkLoggerController | null;
};
//# sourceMappingURL=utils.d.ts.map