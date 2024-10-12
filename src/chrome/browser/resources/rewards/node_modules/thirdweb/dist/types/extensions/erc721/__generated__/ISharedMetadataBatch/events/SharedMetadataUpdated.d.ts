import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "SharedMetadataUpdated" event.
 */
export type SharedMetadataUpdatedEventFilters = Partial<{
    id: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "id";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the SharedMetadataUpdated event.
 * @param filters - Optional filters to apply to the event.
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
 *  sharedMetadataUpdatedEvent({
 *  id: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function sharedMetadataUpdatedEvent(filters?: SharedMetadataUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SharedMetadataUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "id";
        readonly indexed: true;
    }, {
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