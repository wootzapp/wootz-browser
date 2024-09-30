/**
 * Creates an event object for the TransferValidatorUpdated event.
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
 *  RoyaltyERC1155.transferValidatorUpdatedEvent()
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