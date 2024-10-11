import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokensMinted" event.
 */
export type TokensMintedEventFilters = Partial<{
    mintedTo: AbiParameterToPrimitiveType<{
        type: "address";
        name: "mintedTo";
        indexed: true;
    }>;
    tokenIdMinted: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenIdMinted";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TokensMinted event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokensMintedEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokensMintedEvent({
 *  mintedTo: ...,
 *  tokenIdMinted: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokensMintedEvent(filters?: TokensMintedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokensMinted";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "mintedTo";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenIdMinted";
        readonly indexed: true;
    }, {
        readonly type: "string";
        readonly name: "uri";
    }];
}>;
//# sourceMappingURL=TokensMinted.d.ts.map