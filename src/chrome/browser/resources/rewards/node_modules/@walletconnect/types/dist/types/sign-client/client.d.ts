/// <reference types="node" />
import { Logger } from "@walletconnect/logger";
import EventEmmiter from "events";
import { CoreTypes, ICore } from "../core/core";
import { IEngine } from "./engine";
import { IPendingRequest } from "./pendingRequest";
import { IProposal, ProposalTypes } from "./proposal";
import { ISession, SessionTypes } from "./session";
import { Verify } from "../core/verify";
import { IAuth, AuthTypes } from "./auth";
import { RelayerTypes } from "../core";
export declare namespace SignClientTypes {
    type Event = "session_proposal" | "session_update" | "session_extend" | "session_ping" | "session_delete" | "session_expire" | "session_request" | "session_request_sent" | "session_event" | "session_authenticate" | "proposal_expire" | "session_request_expire";
    interface BaseEventArgs<T = unknown> {
        id: number;
        topic: string;
        params: T;
    }
    interface EventArguments {
        session_proposal: {
            verifyContext: Verify.Context;
        } & Omit<BaseEventArgs<ProposalTypes.Struct>, "topic">;
        session_update: BaseEventArgs<{
            namespaces: SessionTypes.Namespaces;
        }>;
        session_extend: Omit<BaseEventArgs, "params">;
        session_ping: Omit<BaseEventArgs, "params">;
        session_delete: Omit<BaseEventArgs, "params">;
        session_expire: {
            topic: string;
        };
        session_request: {
            verifyContext: Verify.Context;
        } & BaseEventArgs<{
            request: {
                method: string;
                params: any;
                expiryTimestamp?: number;
            };
            chainId: string;
        }>;
        session_request_sent: {
            request: {
                method: string;
                params: any;
            };
            topic: string;
            chainId: string;
            id: number;
        };
        session_event: BaseEventArgs<{
            event: {
                name: string;
                data: any;
            };
            chainId: string;
        }>;
        session_authenticate: {
            verifyContext: Verify.Context;
            transportType?: RelayerTypes.TransportType;
        } & BaseEventArgs<AuthTypes.AuthRequestEventArgs>;
        proposal_expire: {
            id: number;
        };
        session_request_expire: {
            id: number;
        };
    }
    type Metadata = CoreTypes.Metadata;
    type SignConfig = {
        disableRequestQueue?: boolean;
    };
    interface Options extends CoreTypes.Options {
        core?: ICore;
        metadata?: Metadata;
        signConfig?: SignConfig;
    }
}
export declare abstract class ISignClientEvents extends EventEmmiter {
    constructor();
    abstract emit: <E extends SignClientTypes.Event>(event: E, args: SignClientTypes.EventArguments[E]) => boolean;
    abstract on: <E extends SignClientTypes.Event>(event: E, listener: (args: SignClientTypes.EventArguments[E]) => any) => this;
    abstract once: <E extends SignClientTypes.Event>(event: E, listener: (args: SignClientTypes.EventArguments[E]) => any) => this;
    abstract off: <E extends SignClientTypes.Event>(event: E, listener: (args: SignClientTypes.EventArguments[E]) => any) => this;
    abstract removeListener: <E extends SignClientTypes.Event>(event: E, listener: (args: SignClientTypes.EventArguments[E]) => any) => this;
    abstract removeAllListeners: <E extends SignClientTypes.Event>(event: E) => this;
}
export declare abstract class ISignClient {
    opts?: SignClientTypes.Options | undefined;
    readonly protocol = "wc";
    readonly version = 2;
    abstract readonly name: string;
    abstract readonly context: string;
    abstract readonly metadata: SignClientTypes.Metadata;
    abstract core: ICore;
    abstract logger: Logger;
    abstract events: ISignClientEvents;
    abstract engine: IEngine;
    abstract session: ISession;
    abstract proposal: IProposal;
    abstract pendingRequest: IPendingRequest;
    abstract auth: IAuth;
    abstract signConfig?: SignClientTypes.SignConfig;
    constructor(opts?: SignClientTypes.Options | undefined);
    abstract connect: IEngine["connect"];
    abstract pair: IEngine["pair"];
    abstract approve: IEngine["approve"];
    abstract reject: IEngine["reject"];
    abstract update: IEngine["update"];
    abstract extend: IEngine["extend"];
    abstract request: IEngine["request"];
    abstract respond: IEngine["respond"];
    abstract ping: IEngine["ping"];
    abstract emit: IEngine["emit"];
    abstract disconnect: IEngine["disconnect"];
    abstract find: IEngine["find"];
    abstract getPendingSessionRequests: IEngine["getPendingSessionRequests"];
    abstract authenticate: IEngine["authenticate"];
    abstract formatAuthMessage: IEngine["formatAuthMessage"];
    abstract approveSessionAuthenticate: IEngine["approveSessionAuthenticate"];
    abstract rejectSessionAuthenticate: IEngine["rejectSessionAuthenticate"];
}
//# sourceMappingURL=client.d.ts.map