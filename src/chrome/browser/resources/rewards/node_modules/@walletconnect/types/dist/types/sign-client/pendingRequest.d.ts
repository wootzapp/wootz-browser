import { IStore, Verify } from "../core";
import { SignClientTypes } from "./";
export declare namespace PendingRequestTypes {
    interface Struct {
        topic: string;
        id: number;
        params: SignClientTypes.EventArguments["session_request"]["params"];
        verifyContext: Verify.Context;
    }
}
export declare type IPendingRequest = IStore<number, PendingRequestTypes.Struct>;
//# sourceMappingURL=pendingRequest.d.ts.map