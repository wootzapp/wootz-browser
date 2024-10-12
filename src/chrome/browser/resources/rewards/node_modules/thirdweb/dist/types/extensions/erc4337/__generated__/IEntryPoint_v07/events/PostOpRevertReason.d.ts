import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PostOpRevertReason" event.
 */
export type PostOpRevertReasonEventFilters = Partial<{
    userOpHash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "userOpHash";
        indexed: true;
    }>;
    sender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "sender";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PostOpRevertReason event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { postOpRevertReasonEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  postOpRevertReasonEvent({
 *  userOpHash: ...,
 *  sender: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function postOpRevertReasonEvent(filters?: PostOpRevertReasonEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PostOpRevertReason";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "bytes32";
        readonly name: "userOpHash";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "sender";
        readonly indexed: true;
    }, {
        readonly type: "uint256";
        readonly name: "nonce";
    }, {
        readonly type: "bytes";
        readonly name: "revertReason";
    }];
}>;
//# sourceMappingURL=PostOpRevertReason.d.ts.map