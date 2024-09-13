import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "SharedMetadataDeleted" event.
 */
export type SharedMetadataDeletedEventFilters = Partial<{
    id: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "id";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the SharedMetadataDeleted event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { sharedMetadataDeletedEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  sharedMetadataDeletedEvent({
 *  id: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function sharedMetadataDeletedEvent(filters?: SharedMetadataDeletedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SharedMetadataDeleted";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "id";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=SharedMetadataDeleted.d.ts.map