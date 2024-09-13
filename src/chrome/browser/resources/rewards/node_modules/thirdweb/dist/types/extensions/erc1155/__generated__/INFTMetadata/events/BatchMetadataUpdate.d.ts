/**
 * Creates an event object for the BatchMetadataUpdate event.
 * @returns The prepared event object.
 * @extension ERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { batchMetadataUpdateEvent } from "thirdweb/extensions/erc1155";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  batchMetadataUpdateEvent()
 * ],
 * });
 * ```
 */
export declare function batchMetadataUpdateEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "BatchMetadataUpdate";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "_fromTokenId";
    }, {
        readonly type: "uint256";
        readonly name: "_toTokenId";
    }];
}>;
//# sourceMappingURL=BatchMetadataUpdate.d.ts.map