import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "NewMetadataBatch" event.
 */
export type NewMetadataBatchEventFilters = Partial<{
    startTokenIdInclusive: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "startTokenIdInclusive";
        indexed: true;
    }>;
    endTokenIdNonInclusive: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "endTokenIdNonInclusive";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the NewMetadataBatch event.
 * @param filters - Optional filters to apply to the event.
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
 *  MintableERC721.newMetadataBatchEvent({
 *  startTokenIdInclusive: ...,
 *  endTokenIdNonInclusive: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function newMetadataBatchEvent(filters?: NewMetadataBatchEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "NewMetadataBatch";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "startTokenIdInclusive";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "endTokenIdNonInclusive";
        readonly indexed: true;
    }, {
        readonly type: "string";
        readonly name: "baseURI";
    }];
}>;
//# sourceMappingURL=NewMetadataBatch.d.ts.map