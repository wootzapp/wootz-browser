import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PackCreated" event.
 */
export type PackCreatedEventFilters = Partial<{
    packId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "packId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PackCreated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { packCreatedEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  packCreatedEvent({
 *  packId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function packCreatedEvent(filters?: PackCreatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PackCreated";
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
//# sourceMappingURL=PackCreated.d.ts.map