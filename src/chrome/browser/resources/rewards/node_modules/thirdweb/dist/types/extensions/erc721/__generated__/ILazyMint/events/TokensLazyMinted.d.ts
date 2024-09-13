import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokensLazyMinted" event.
 */
export type TokensLazyMintedEventFilters = Partial<{
    startTokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "startTokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TokensLazyMinted event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokensLazyMintedEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokensLazyMintedEvent({
 *  startTokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokensLazyMintedEvent(filters?: TokensLazyMintedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokensLazyMinted";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "startTokenId";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "endTokenId";
    }, {
        readonly type: "string";
        readonly name: "baseURI";
    }, {
        readonly type: "bytes";
        readonly name: "encryptedBaseURI";
    }];
}>;
//# sourceMappingURL=TokensLazyMinted.d.ts.map