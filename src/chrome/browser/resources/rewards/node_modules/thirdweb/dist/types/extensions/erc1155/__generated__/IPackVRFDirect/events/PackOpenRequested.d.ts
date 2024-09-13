import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PackOpenRequested" event.
 */
export type PackOpenRequestedEventFilters = Partial<{
    opener: AbiParameterToPrimitiveType<{
        type: "address";
        name: "opener";
        indexed: true;
    }>;
    packId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "packId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PackOpenRequested event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { packOpenRequestedEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  packOpenRequestedEvent({
 *  opener: ...,
 *  packId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function packOpenRequestedEvent(filters?: PackOpenRequestedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PackOpenRequested";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "opener";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "packId";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "amountToOpen";
    }, {
        readonly type: "uint256";
        readonly name: "requestId";
    }];
}>;
//# sourceMappingURL=PackOpenRequested.d.ts.map