/**
 * Creates an event object for the BatchMetadataUpdate event.
 * @returns The prepared event object.
 * @modules MintableERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { MintableERC721 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  MintableERC721.batchMetadataUpdateEvent()
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