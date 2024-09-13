/**
 * Creates an event object for the Airdrop event.
 * @returns The prepared event object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { airdropEvent } from "thirdweb/extensions/airdrop";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  airdropEvent()
 * ],
 * });
 * ```
 */
export declare function airdropEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "Airdrop";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "token";
    }];
}>;
//# sourceMappingURL=Airdrop.d.ts.map