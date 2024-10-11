import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokensStaked" event.
 */
export type TokensStakedEventFilters = Partial<{
    staker: AbiParameterToPrimitiveType<{
        type: "address";
        name: "staker";
        indexed: true;
    }>;
    tokenId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "tokenId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TokensStaked event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokensStakedEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokensStakedEvent({
 *  staker: ...,
 *  tokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokensStakedEvent(filters?: TokensStakedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokensStaked";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "staker";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "tokenId";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "amount";
    }];
}>;
//# sourceMappingURL=TokensStaked.d.ts.map