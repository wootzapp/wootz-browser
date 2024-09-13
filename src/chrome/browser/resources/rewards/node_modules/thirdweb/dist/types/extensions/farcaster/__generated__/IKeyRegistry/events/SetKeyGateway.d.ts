/**
 * Creates an event object for the SetKeyGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setKeyGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setKeyGatewayEvent()
 * ],
 * });
 * ```
 */
export declare function setKeyGatewayEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SetKeyGateway";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "oldKeyGateway";
    }, {
        readonly type: "address";
        readonly name: "newKeyGateway";
    }];
}>;
//# sourceMappingURL=SetKeyGateway.d.ts.map