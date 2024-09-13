import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PackRandomnessFulfilled" event.
 */
export type PackRandomnessFulfilledEventFilters = Partial<{
    packId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "packId";
        indexed: true;
    }>;
    requestId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "requestId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PackRandomnessFulfilled event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { packRandomnessFulfilledEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  packRandomnessFulfilledEvent({
 *  packId: ...,
 *  requestId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function packRandomnessFulfilledEvent(filters?: PackRandomnessFulfilledEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PackRandomnessFulfilled";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "packId";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "requestId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=PackRandomnessFulfilled.d.ts.map