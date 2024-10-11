/**
 * Creates an event object for the FreezeIdGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { freezeIdGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  freezeIdGatewayEvent()
 * ],
 * });
 * ```
 */
export declare function freezeIdGatewayEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "FreezeIdGateway";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "idGateway";
    }];
}>;
//# sourceMappingURL=FreezeIdGateway.d.ts.map