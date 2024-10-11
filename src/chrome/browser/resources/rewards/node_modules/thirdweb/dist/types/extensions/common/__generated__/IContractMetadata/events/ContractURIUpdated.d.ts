/**
 * Creates an event object for the ContractURIUpdated event.
 * @returns The prepared event object.
 * @extension COMMON
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { contractURIUpdatedEvent } from "thirdweb/extensions/common";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  contractURIUpdatedEvent()
 * ],
 * });
 * ```
 */
export declare function contractURIUpdatedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ContractURIUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "string";
        readonly name: "prevURI";
    }, {
        readonly type: "string";
        readonly name: "newURI";
    }];
}>;
//# sourceMappingURL=ContractURIUpdated.d.ts.map