import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ClaimConditionsUpdated" event.
 */
export type ClaimConditionsUpdatedEventFilters = Partial<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ClaimConditionsUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { claimConditionsUpdatedEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  claimConditionsUpdatedEvent({
 *  tokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function claimConditionsUpdatedEvent(filters?: ClaimConditionsUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ClaimConditionsUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "tokenId";
        readonly indexed: true;
    }, {
        readonly type: "tuple[]";
        readonly components: readonly [{
            readonly type: "uint256";
            readonly name: "startTimestamp";
        }, {
            readonly type: "uint256";
            readonly name: "maxClaimableSupply";
        }, {
            readonly type: "uint256";
            readonly name: "supplyClaimed";
        }, {
            readonly type: "uint256";
            readonly name: "quantityLimitPerWallet";
        }, {
            readonly type: "bytes32";
            readonly name: "merkleRoot";
        }, {
            readonly type: "uint256";
            readonly name: "pricePerToken";
        }, {
            readonly type: "address";
            readonly name: "currency";
        }, {
            readonly type: "string";
            readonly name: "metadata";
        }];
        readonly name: "claimConditions";
    }, {
        readonly type: "bool";
        readonly name: "resetEligibility";
    }];
}>;
//# sourceMappingURL=ClaimConditionsUpdated.d.ts.map