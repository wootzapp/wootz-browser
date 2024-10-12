import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ContractUnpublished" event.
 */
export type ContractUnpublishedEventFilters = Partial<{
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
    contractId: AbiParameterToPrimitiveType<{
        type: "string";
        name: "contractId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ContractUnpublished event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { contractUnpublishedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  contractUnpublishedEvent({
 *  operator: ...,
 *  publisher: ...,
 *  contractId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function contractUnpublishedEvent(filters?: ContractUnpublishedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ContractUnpublished";
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
        readonly type: "string";
        readonly name: "contractId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=ContractUnpublished.d.ts.map