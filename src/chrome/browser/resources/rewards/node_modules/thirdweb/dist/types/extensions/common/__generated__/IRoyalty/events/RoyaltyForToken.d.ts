import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RoyaltyForToken" event.
 */
export type RoyaltyForTokenEventFilters = Partial<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
        indexed: true;
    }>;
    royaltyRecipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "royaltyRecipient";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RoyaltyForToken event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { royaltyForTokenEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  royaltyForTokenEvent({
 *  tokenId: ...,
 *  royaltyRecipient: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function royaltyForTokenEvent(filters?: RoyaltyForTokenEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RoyaltyForToken";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "tokenId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "royaltyRecipient";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "royaltyBps";
    }];
}>;
//# sourceMappingURL=RoyaltyForToken.d.ts.map