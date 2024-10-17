import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "RuleCreated" event.
 */
export type RuleCreatedEventFilters = Partial<{
    ruleId: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "ruleId";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the RuleCreated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { ruleCreatedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  ruleCreatedEvent({
 *  ruleId: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function ruleCreatedEvent(filters?: RuleCreatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "RuleCreated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "ruleId";
        readonly indexed: true;
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "bytes32";
            readonly name: "ruleId";
        }, {
            readonly type: "address";
            readonly name: "token";
        }, {
            readonly type: "uint8";
            readonly name: "tokenType";
        }, {
            readonly type: "uint256";
            readonly name: "tokenId";
        }, {
            readonly type: "uint256";
            readonly name: "balance";
        }, {
            readonly type: "uint256";
            readonly name: "score";
        }, {
            readonly type: "uint8";
            readonly name: "ruleType";
        }];
        readonly name: "rule";
    }];
}>;
//# sourceMappingURL=RuleCreated.d.ts.map