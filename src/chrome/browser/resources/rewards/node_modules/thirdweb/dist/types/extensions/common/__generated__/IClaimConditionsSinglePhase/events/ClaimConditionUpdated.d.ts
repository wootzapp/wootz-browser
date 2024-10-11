/**
 * Creates an event object for the ClaimConditionUpdated event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { claimConditionUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  claimConditionUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function claimConditionUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ClaimConditionUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "tuple";
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
        readonly name: "resetClaimEligibility";
    }];
}>;
//# sourceMappingURL=ClaimConditionUpdated.d.ts.map