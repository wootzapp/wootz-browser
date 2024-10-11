import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PackOpened" event.
 */
export type PackOpenedEventFilters = Partial<{
    packId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "packId";
        indexed: true;
    }>;
    opener: AbiParameterToPrimitiveType<{
        type: "address";
        name: "opener";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PackOpened event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { packOpenedEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  packOpenedEvent({
 *  packId: ...,
 *  opener: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function packOpenedEvent(filters?: PackOpenedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PackOpened";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "packId";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "opener";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "numOfPacksOpened";
    }, {
        readonly type: "tuple[]";
        readonly components: readonly [{
            readonly type: "address";
            readonly name: "assetContract";
        }, {
            readonly type: "uint8";
            readonly name: "tokenType";
        }, {
            readonly type: "uint256";
            readonly name: "tokenId";
        }, {
            readonly type: "uint256";
            readonly name: "totalAmount";
        }];
        readonly name: "rewardUnitsDistributed";
    }];
}>;
//# sourceMappingURL=PackOpened.d.ts.map