import { Logger } from "@walletconnect/logger";
import { ICore, IEventClient } from "@walletconnect/types";
export declare class EventClient extends IEventClient {
    core: ICore;
    logger: Logger;
    readonly context = "event-client";
    private readonly storagePrefix;
    private readonly storageVersion;
    private events;
    private shouldPersist;
    constructor(core: ICore, logger: Logger, telemetryEnabled?: boolean);
    get storageKey(): string;
    init: IEventClient["init"];
    createEvent: IEventClient["createEvent"];
    getEvent: IEventClient["getEvent"];
    deleteEvent: IEventClient["deleteEvent"];
    private setEventListeners;
    private setMethods;
    private addTrace;
    private setError;
    private persist;
    private restore;
    private submit;
    private sendEvent;
    private getAppDomain;
}
//# sourceMappingURL=events.d.ts.map