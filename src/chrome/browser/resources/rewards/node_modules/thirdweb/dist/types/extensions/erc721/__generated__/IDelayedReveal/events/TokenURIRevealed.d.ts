import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokenURIRevealed" event.
 */
export type TokenURIRevealedEventFilters = Partial<{
    index: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "index";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TokenURIRevealed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokenURIRevealedEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokenURIRevealedEvent({
 *  index: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokenURIRevealedEvent(filters?: TokenURIRevealedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokenURIRevealed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "index";
        readonly indexed: true;
    }, {
        readonly type: "string";
        readonly name: "revealedURI";
    }];
}>;
//# sourceMappingURL=TokenURIRevealed.d.ts.map