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
    approved: AbiParameterToPrimitiveType<{
        type: "address";
        name: "approved";
        indexed: true;
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Approval event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { approvalEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  approvalEvent({
 *  owner: ...,
 *  approved: ...,
 *  tokenId: ...,
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
        readonly name: "approved";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=Approval.d.ts.map