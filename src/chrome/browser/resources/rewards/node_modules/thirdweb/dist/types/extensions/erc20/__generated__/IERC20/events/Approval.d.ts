import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Approval" event.
 */
export type ApprovalEventFilters = Partial<{
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
        indexed: true;
    }>;
    spender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "spender";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Approval event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC20
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { approvalEvent } from "thirdweb/extensions/erc20";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  approvalEvent({
 *  owner: ...,
 *  spender: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function approvalEvent(filters?: ApprovalEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Approval";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "owner";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "spender";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "value";
    }];
}>;
//# sourceMappingURL=Approval.d.ts.map