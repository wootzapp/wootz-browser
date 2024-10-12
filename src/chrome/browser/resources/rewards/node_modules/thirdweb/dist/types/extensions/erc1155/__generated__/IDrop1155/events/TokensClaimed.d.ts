import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokensClaimed" event.
 */
export type TokensClaimedEventFilters = Partial<{
    claimConditionIndex: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "claimConditionIndex";
        indexed: true;
    }>;
    claimer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "claimer";
        indexed: true;
    }>;
    receiver: AbiParameterToPrimitiveType<{
        type: "address";
        name: "receiver";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TokensClaimed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokensClaimedEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokensClaimedEvent({
 *  claimConditionIndex: ...,
 *  claimer: ...,
 *  receiver: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokensClaimedEvent(filters?: TokensClaimedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokensClaimed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "claimConditionIndex";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "claimer";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "receiver";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
    }, {
        readonly type: "uint256";
        readonly name: "quantityClaimed";
    }];
}>;
//# sourceMappingURL=TokensClaimed.d.ts.map