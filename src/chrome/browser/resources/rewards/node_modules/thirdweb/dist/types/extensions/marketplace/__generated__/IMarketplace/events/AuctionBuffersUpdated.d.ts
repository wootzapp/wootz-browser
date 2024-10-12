/**
 * Creates an event object for the AuctionBuffersUpdated event.
 * @returns The prepared event object.
 * @extension MARKETPLACE
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { auctionBuffersUpdatedEvent } from "thirdweb/extensions/marketplace";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  auctionBuffersUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function auctionBuffersUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AuctionBuffersUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "timeBuffer";
    }, {
        readonly type: "uint256";
        readonly name: "bidBufferBps";
    }];
}>;
//# sourceMappingURL=AuctionBuffersUpdated.d.ts.map