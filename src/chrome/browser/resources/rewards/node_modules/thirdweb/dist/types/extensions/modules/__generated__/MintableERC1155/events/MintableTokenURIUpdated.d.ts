/**
 * Creates an event object for the MintableTokenURIUpdated event.
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
 *  MintableERC1155.mintableTokenURIUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function mintableTokenURIUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "MintableTokenURIUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "tokenId";
    }, {
        readonly type: "string";
        readonly name: "tokenURI";
    }];
}>;
//# sourceMappingURL=MintableTokenURIUpdated.d.ts.map