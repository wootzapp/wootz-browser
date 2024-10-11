import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "SignerRemoved" event.
 */
export type SignerRemovedEventFilters = Partial<{
    account: AbiParameterToPrimitiveType<{
        type: "address";
        name: "account";
        indexed: true;
    }>;
    signer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "signer";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the SignerRemoved event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { signerRemovedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  signerRemovedEvent({
 *  account: ...,
 *  signer: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function signerRemovedEvent(filters?: SignerRemovedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SignerRemoved";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "account";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "signer";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=SignerRemoved.d.ts.map