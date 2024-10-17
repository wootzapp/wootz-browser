import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "TokensWithdrawn" event.
 */
export type TokensWithdrawnEventFilters = Partial<{
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
 * Creates an event object for the TokensWithdrawn event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokensWithdrawnEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokensWithdrawnEvent({
 *  staker: ...,
 *  tokenId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function tokensWithdrawnEvent(filters?: TokensWithdrawnEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "TokensWithdrawn";
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
//# sourceMappingURL=TokensWithdrawn.d.ts.map