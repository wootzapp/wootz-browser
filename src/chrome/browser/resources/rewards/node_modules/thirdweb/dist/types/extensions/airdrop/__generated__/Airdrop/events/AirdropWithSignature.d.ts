/**
 * Creates an event object for the AirdropWithSignature event.
 * @returns The prepared event object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { airdropWithSignatureEvent } from "thirdweb/extensions/airdrop";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  airdropWithSignatureEvent()
 * ],
 * });
 * ```
 */
export declare function airdropWithSignatureEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AirdropWithSignature";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "token";
    }];
}>;
//# sourceMappingURL=AirdropWithSignature.d.ts.map