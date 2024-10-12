import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RulesEngineOverriden" event.
 */
export type RulesEngineOverridenEventFilters = Partial<{
    newRulesEngine: AbiParameterToPrimitiveType<{
        type: "address";
        name: "newRulesEngine";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RulesEngineOverriden event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { rulesEngineOverridenEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  rulesEngineOverridenEvent({
 *  newRulesEngine: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function rulesEngineOverridenEvent(filters?: RulesEngineOverridenEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RulesEngineOverriden";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "newRulesEngine";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=RulesEngineOverriden.d.ts.map