/**
 * Creates an event object for the MetadataUpdate event.
 * @returns The prepared event object.
 * @modules MintableERC1155
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { MintableERC1155 } from "thirdweb/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  MintableERC1155.metadataUpdateEvent()
 * ],
 * });
 * ```
 */
export declare function metadataUpdateEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "MetadataUpdate";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "id";
    }];
}>;
//# sourceMappingURL=MetadataUpdate.d.ts.map