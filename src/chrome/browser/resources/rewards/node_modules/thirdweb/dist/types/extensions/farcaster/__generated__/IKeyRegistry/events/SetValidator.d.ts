/**
 * Creates an event object for the SetValidator event.
 * @returns The prepared event object.
 * @extension FARCASTER
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { setValidatorEvent } from "thirdweb/extensions/farcaster";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  setValidatorEvent()
 * ],
 * });
 * ```
 */
export declare function setValidatorEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SetValidator";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "uint32";
        readonly name: "keyType";
    }, {
        readonly type: "uint8";
        readonly name: "metadataType";
    }, {
        readonly type: "address";
        readonly name: "oldValidator";
    }, {
        readonly type: "address";
        readonly name: "newValidator";
    }];
}>;
//# sourceMappingURL=SetValidator.d.ts.map