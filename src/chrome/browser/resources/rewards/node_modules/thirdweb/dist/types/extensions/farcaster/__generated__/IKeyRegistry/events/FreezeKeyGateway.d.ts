/**
 * Creates an event object for the FreezeKeyGateway event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { freezeKeyGatewayEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  freezeKeyGatewayEvent()
 * ],
 * });
 * ```
 */
export declare function freezeKeyGatewayEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "FreezeKeyGateway";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "keyGateway";
    }];
}>;
//# sourceMappingURL=FreezeKeyGateway.d.ts.map