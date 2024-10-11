import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "DelegateChanged" event.
 */
export type DelegateChangedEventFilters = Partial<{
    delegator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "delegator";
        indexed: true;
    }>;
    fromDelegate: AbiParameterToPrimitiveType<{
        type: "address";
        name: "fromDelegate";
        indexed: true;
    }>;
    toDelegate: AbiParameterToPrimitiveType<{
        type: "address";
        name: "toDelegate";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the DelegateChanged event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { delegateChangedEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  delegateChangedEvent({
 *  delegator: ...,
 *  fromDelegate: ...,
 *  toDelegate: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function delegateChangedEvent(filters?: DelegateChangedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "DelegateChanged";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "delegator";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "fromDelegate";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "toDelegate";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=DelegateChanged.d.ts.map