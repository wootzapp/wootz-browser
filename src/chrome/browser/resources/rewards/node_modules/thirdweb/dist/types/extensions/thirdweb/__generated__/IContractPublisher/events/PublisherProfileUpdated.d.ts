import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PublisherProfileUpdated" event.
 */
export type PublisherProfileUpdatedEventFilters = Partial<{
    publisher: AbiParameterToPrimitiveType<{
        type: "address";
        name: "publisher";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PublisherProfileUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { publisherProfileUpdatedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  publisherProfileUpdatedEvent({
 *  publisher: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function publisherProfileUpdatedEvent(filters?: PublisherProfileUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PublisherProfileUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "publisher";
        readonly indexed: true;
    }, {
        readonly type: "string";
        readonly name: "prevURI";
    }, {
        readonly type: "string";
        readonly name: "newURI";
    }];
}>;
//# sourceMappingURL=PublisherProfileUpdated.d.ts.map