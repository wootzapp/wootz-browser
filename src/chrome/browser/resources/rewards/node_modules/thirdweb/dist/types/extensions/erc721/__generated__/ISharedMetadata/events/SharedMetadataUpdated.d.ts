/**
 * Creates an event object for the SharedMetadataUpdated event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { sharedMetadataUpdatedEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  sharedMetadataUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function sharedMetadataUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SharedMetadataUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "string";
        readonly name: "name";
    }, {
        readonly type: "string";
        readonly name: "description";
    }, {
        readonly type: "string";
        readonly name: "imageURI";
    }, {
        readonly type: "string";
        readonly name: "animationURI";
    }];
}>;
//# sourceMappingURL=SharedMetadataUpdated.d.ts.map