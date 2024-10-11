/**
 * Creates an event object for the MetadataFrozen event.
 * @returns The prepared event object.
 * @extension ERC721
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { metadataFrozenEvent } from "thirdweb/extensions/erc721";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  metadataFrozenEvent()
 * ],
 * });
 * ```
 */
export declare function metadataFrozenEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "MetadataFrozen";
    readonly type: "event";
    readonly inputs: readonly [];
}>;
//# sourceMappingURL=MetadataFrozen.d.ts.map