import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ContractPublished" event.
 */
export type ContractPublishedEventFilters = Partial<{
    operator: AbiParameterToPrimitiveType<{
        type: "address";
        name: "operator";
        indexed: true;
    }>;
    publisher: AbiParameterToPrimitiveType<{
        type: "address";
        name: "publisher";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ContractPublished event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { contractPublishedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  contractPublishedEvent({
 *  operator: ...,
 *  publisher: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function contractPublishedEvent(filters?: ContractPublishedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ContractPublished";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "operator";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "publisher";
        readonly indexed: true;
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "string";
            readonly name: "contractId";
        }, {
            readonly type: "uint256";
            readonly name: "publishTimestamp";
        }, {
            readonly type: "string";
            readonly name: "publishMetadataUri";
        }, {
            readonly type: "bytes32";
            readonly name: "bytecodeHash";
        }, {
            readonly type: "address";
            readonly name: "implementation";
        }];
        readonly name: "publishedContract";
    }];
}>;
//# sourceMappingURL=ContractPublished.d.ts.map