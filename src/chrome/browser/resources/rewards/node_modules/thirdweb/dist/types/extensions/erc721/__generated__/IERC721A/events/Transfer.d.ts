import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Transfer" event.
 */
export type TransferEventFilters = Partial<{
    from: AbiParameterToPrimitiveType<{
        type: "address";
        name: "from";
        indexed: true;
    }>;
    to: AbiParameterToPrimitiveType<{
        type: "address";
        name: "to";
        indexed: true;
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Transfer event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { transferEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  transferEvent({
 *  from: ...,
 *  to: ...,
 *  tokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function transferEvent(filters?: TransferEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Transfer";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "from";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "to";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=Transfer.d.ts.map