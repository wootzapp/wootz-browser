/**
 * Creates an event object for the MetadataUpdate event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { metadataUpdateEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  metadataUpdateEvent()
 * ],
 * });
 * ```
 */
export declare function metadataUpdateEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "MetadataUpdate";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "_tokenId";
    }];
}>;
//# sourceMappingURL=MetadataUpdate.d.ts.map