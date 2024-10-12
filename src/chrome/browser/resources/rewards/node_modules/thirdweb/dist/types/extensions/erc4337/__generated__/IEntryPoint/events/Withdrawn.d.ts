import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "Withdrawn" event.
 */
export type WithdrawnEventFilters = Partial<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the Withdrawn event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { withdrawnEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  withdrawnEvent({
 *  account: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function withdrawnEvent(filters?: WithdrawnEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Withdrawn";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "withdrawAddress";
    }, {
        readonly type: "uint256";
        readonly name: "amount";
    }];
}>;
//# sourceMappingURL=Withdrawn.d.ts.map