import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "DelegateVotesChanged" event.
 */
export type DelegateVotesChangedEventFilters = Partial<{
    delegate: AbiParameterToPrimitiveType<{
        type: "address";
        name: "delegate";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the DelegateVotesChanged event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { delegateVotesChangedEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  delegateVotesChangedEvent({
 *  delegate: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function delegateVotesChangedEvent(filters?: DelegateVotesChangedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "DelegateVotesChanged";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "delegate";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "previousBalance";
    }, {
        readonly type: "uint256";
        readonly name: "newBalance";
    }];
}>;
//# sourceMappingURL=DelegateVotesChanged.d.ts.map