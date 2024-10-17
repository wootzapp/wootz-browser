import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RuleDeleted" event.
 */
export type RuleDeletedEventFilters = Partial<{
    ruleId: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "ruleId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RuleDeleted event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { ruleDeletedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  ruleDeletedEvent({
 *  ruleId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function ruleDeletedEvent(filters?: RuleDeletedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RuleDeleted";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "ruleId";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=RuleDeleted.d.ts.map