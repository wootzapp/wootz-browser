import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Deleted" event.
 */
export type DeletedEventFilters = Partial<{
    deployer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "deployer";
        indexed: true;
    }>;
    deployment: AbiParameterToPrimitiveType<{
        type: "address";
        name: "deployment";
        indexed: true;
    }>;
    chainId: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "chainId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Deleted event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { deletedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  deletedEvent({
 *  deployer: ...,
 *  deployment: ...,
 *  chainId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function deletedEvent(filters?: DeletedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Deleted";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "deployer";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "deployment";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "chainId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=Deleted.d.ts.map