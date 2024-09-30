import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokenRoyaltyUpdated" event.
 */
export type TokenRoyaltyUpdatedEventFilters = Partial<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
        indexed: true;
    }>;
    recipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recipient";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TokenRoyaltyUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @modules RoyaltyERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { RoyaltyERC1155 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  RoyaltyERC1155.tokenRoyaltyUpdatedEvent({
 *  tokenId: ...,
 *  recipient: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokenRoyaltyUpdatedEvent(filters?: TokenRoyaltyUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokenRoyaltyUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "tokenId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "recipient";
        readonly indexed: true;
    }, {
        readonly type: "uint16";
        readonly name: "bps";
    }];
}>;
//# sourceMappingURL=TokenRoyaltyUpdated.d.ts.map