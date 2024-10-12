import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "AirdropFailed" event.
 */
export type AirdropFailedEventFilters = Partial<{
    tokenAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenAddress";
        indexed: true;
    }>;
    tokenOwner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "tokenOwner";
        indexed: true;
    }>;
    recipient: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recipient";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the AirdropFailed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { airdropFailedEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  airdropFailedEvent({
 *  tokenAddress: ...,
 *  tokenOwner: ...,
 *  recipient: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function airdropFailedEvent(filters?: AirdropFailedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AirdropFailed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "tokenAddress";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "tokenOwner";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "recipient";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
    }];
}>;
//# sourceMappingURL=AirdropFailed.d.ts.map