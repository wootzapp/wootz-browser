import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TransferBatch" event.
 */
export type TransferBatchEventFilters = Partial<{
    _operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_operator";
        indexed: true;
    }>;
    _from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_from";
        indexed: true;
    }>;
    _to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "_to";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TransferBatch event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { transferBatchEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  transferBatchEvent({
 *  _operator: ...,
 *  _from: ...,
 *  _to: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function transferBatchEvent(filters?: TransferBatchEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TransferBatch";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "_operator";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "_from";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "_to";
        readonly indexed: true;
    }, {
        readonly type: "uint256[]";
        readonly name: "tokenIds";
    }, {
        readonly type: "uint256[]";
        readonly name: "_values";
    }];
}>;
//# sourceMappingURL=TransferBatch.d.ts.map