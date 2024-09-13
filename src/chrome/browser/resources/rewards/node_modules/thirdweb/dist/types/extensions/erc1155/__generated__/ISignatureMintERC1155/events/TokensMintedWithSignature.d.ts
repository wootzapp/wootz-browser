import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokensMintedWithSignature" event.
 */
export type TokensMintedWithSignatureEventFilters = Partial<{
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
        indexed: true;
    }>;
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
 * Creates an event object for the TokensMintedWithSignature event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokensMintedWithSignatureEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokensMintedWithSignatureEvent({
 *  signer: ...,
 *  mintedTo: ...,
 *  tokenIdMinted: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokensMintedWithSignatureEvent(filters?: TokensMintedWithSignatureEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokensMintedWithSignature";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "signer";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "mintedTo";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenIdMinted";
        readonly indexed: true;
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "address";
            readonly name: "to";
        }, {
            readonly type: "address";
            readonly name: "royaltyRecipient";
        }, {
            readonly type: "uint256";
            readonly name: "royaltyBps";
        }, {
            readonly type: "address";
            readonly name: "primarySaleRecipient";
        }, {
            readonly type: "uint256";
            readonly name: "tokenId";
        }, {
            readonly type: "string";
            readonly name: "uri";
        }, {
            readonly type: "uint256";
            readonly name: "quantity";
        }, {
            readonly type: "uint256";
            readonly name: "pricePerToken";
        }, {
            readonly type: "address";
            readonly name: "currency";
        }, {
            readonly type: "uint128";
            readonly name: "validityStartTimestamp";
        }, {
            readonly type: "uint128";
            readonly name: "validityEndTimestamp";
        }, {
            readonly type: "bytes32";
            readonly name: "uid";
        }];
        readonly name: "mintRequest";
    }];
}>;
//# sourceMappingURL=TokensMintedWithSignature.d.ts.map