import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ChangeRecoveryAddress" event.
 */
export type ChangeRecoveryAddressEventFilters = Partial<{
    id: AbiParameterToPrimitiveType<{
        type: "uint256";
        name: "id";
        indexed: true;
    }>;
    recovery: AbiParameterToPrimitiveType<{
        type: "address";
        name: "recovery";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ChangeRecoveryAddress event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { changeRecoveryAddressEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  changeRecoveryAddressEvent({
 *  id: ...,
 *  recovery: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function changeRecoveryAddressEvent(filters?: ChangeRecoveryAddressEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ChangeRecoveryAddress";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint256";
        readonly name: "id";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "recovery";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=ChangeRecoveryAddress.d.ts.map