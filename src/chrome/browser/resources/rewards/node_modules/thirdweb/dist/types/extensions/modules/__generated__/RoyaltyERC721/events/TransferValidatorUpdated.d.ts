/**
 * Creates an event object for the TransferValidatorUpdated event.
 * @returns The prepared event object.
 * @modules RoyaltyERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { RoyaltyERC721 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  RoyaltyERC721.transferValidatorUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function transferValidatorUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TransferValidatorUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "oldValidator";
    }, {
        readonly type: "address";
        readonly name: "newValidator";
    }];
}>;
//# sourceMappingURL=TransferValidatorUpdated.d.ts.map