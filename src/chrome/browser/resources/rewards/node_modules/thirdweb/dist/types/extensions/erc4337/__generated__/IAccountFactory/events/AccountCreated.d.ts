import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "AccountCreated" event.
 */
export type AccountCreatedEventFilters = Partial<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
    accountAdmin: AbiParameterToPrimitiveType<{
        type: "address";
        name: "accountAdmin";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the AccountCreated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { accountCreatedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  accountCreatedEvent({
 *  account: ...,
 *  accountAdmin: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function accountCreatedEvent(filters?: AccountCreatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AccountCreated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "accountAdmin";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=AccountCreated.d.ts.map