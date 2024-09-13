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
    tokenIds: AbiParameterToPrimitiveType<{
        type: "uint256[]";
        name: "tokenIds";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the TokensWithdrawn event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { tokensWithdrawnEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  tokensWithdrawnEvent({
 *  staker: ...,
 *  tokenIds: ...,
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
        readonly type: "uint256[]";
        readonly name: "tokenIds";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=TokensWithdrawn.d.ts.map