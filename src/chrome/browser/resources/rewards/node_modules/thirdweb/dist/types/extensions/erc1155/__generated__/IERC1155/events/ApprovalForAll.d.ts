import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ApprovalForAll" event.
 */
export type ApprovalForAllEventFilters = Partial<{
    _owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_owner";
        indexed: true;
    }>;
    _operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_operator";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ApprovalForAll event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { approvalForAllEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  approvalForAllEvent({
 *  _owner: ...,
 *  _operator: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function approvalForAllEvent(filters?: ApprovalForAllEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ApprovalForAll";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "_owner";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "_operator";
        readonly indexed: true;
    }, {
        readonly type: "bool";
        readonly name: "_approved";
    }];
}>;
//# sourceMappingURL=ApprovalForAll.d.ts.map