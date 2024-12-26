import { Logger } from "@walletconnect/logger";
import { ICore } from "./core";
export declare namespace EventClientTypes {
    interface Event {
        eventId: string;
        bundleId: string;
        timestamp: number;
        props: Props;
        addTrace: (trace: string) => void;
        setError: (error: string) => void;
    }
    interface Props {
        event: string;
        type: string;
        properties: Properties;
    }
    interface Properties {
        topic: string;
        trace: Trace;
    }
    type Trace = string[];
}
export declare abstract class IEventClient {
    core: ICore;
    logger: Logger;
    telemetryEnabled: boolean;
    abstract readonly context: string;
    constructor(core: ICore, logger: Logger, telemetryEnabled: boolean);
    abstract init(): Promise<void>;
    abstract createEvent(params: {
        event?: "ERROR";
        type?: string;
        properties: {
            topic: string;
            trace: EventClientTypes.Trace;
        };
    }): EventClientTypes.Event;
    abstract getEvent(params: {
        eventId?: string;
        topic?: string;
    }): EventClientTypes.Event | undefined;
    abstract deleteEvent(params: {
        eventId: string;
    }): void;
}
//# sourceMappingURL=events.d.ts.map