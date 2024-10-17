import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ApprovalForAll" event.
 */
export type ApprovalForAllEventFilters = Partial<{
    owner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "owner";
        indexed: true;
    }>;
    operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "operator";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ApprovalForAll event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { approvalForAllEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  approvalForAllEvent({
 *  owner: ...,
 *  operator: ...,
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
        readonly name: "owner";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "operator";
        readonly indexed: true;
    }, {
        readonly type: "bool";
        readonly name: "approved";
    }];
}>;
//# sourceMappingURL=ApprovalForAll.d.ts.map