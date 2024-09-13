/**
 * Creates an event object for the SetIdGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setIdGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setIdGatewayEvent()
 * ],
 * });
 * ```
 */
export declare function setIdGatewayEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SetIdGateway";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "oldIdGateway";
    }, {
        readonly type: "address";
        readonly name: "newIdGateway";
    }];
}>;
//# sourceMappingURL=SetIdGateway.d.ts.map