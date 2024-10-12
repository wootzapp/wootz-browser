import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "URI" event.
 */
export type URIEventFilters = Partial<{
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the URI event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { uRIEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  uRIEvent({
 *  tokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function uRIEvent(filters?: URIEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "URI";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "string";
        readonly name: "_value";
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=URI.d.ts.map