import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PackUpdated" event.
 */
export type PackUpdatedEventFilters = Partial<{
    packId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "packId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PackUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { packUpdatedEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  packUpdatedEvent({
 *  packId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function packUpdatedEvent(filters?: PackUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PackUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "packId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "recipient";
    }, {
        readonly type: "uint256";
        readonly name: "totalPacksCreated";
    }];
}>;
//# sourceMappingURL=PackUpdated.d.ts.map