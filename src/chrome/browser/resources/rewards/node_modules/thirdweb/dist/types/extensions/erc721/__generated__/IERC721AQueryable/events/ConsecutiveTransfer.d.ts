import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ConsecutiveTransfer" event.
 */
export type ConsecutiveTransferEventFilters = Partial<{
    fromTokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "fromTokenId";
        indexed: true;
    }>;
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
        indexed: true;
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ConsecutiveTransfer event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { consecutiveTransferEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  consecutiveTransferEvent({
 *  fromTokenId: ...,
 *  from: ...,
 *  to: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function consecutiveTransferEvent(filters?: ConsecutiveTransferEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ConsecutiveTransfer";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "fromTokenId";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "toTokenId";
    }, {
        readonly type: "address";
        readonly name: "from";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "to";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=ConsecutiveTransfer.d.ts.map