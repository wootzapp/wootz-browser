/**
 * Creates an event object for the ClaimConditionsUpdated event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { claimConditionsUpdatedEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  claimConditionsUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function claimConditionsUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ClaimConditionsUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
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