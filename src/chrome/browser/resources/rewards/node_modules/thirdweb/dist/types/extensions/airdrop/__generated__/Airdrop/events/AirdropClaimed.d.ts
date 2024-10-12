/**
 * Creates an event object for the AirdropClaimed event.
 * @returns The prepared event object.
 * @extension AIRDROP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { airdropClaimedEvent } from "thirdweb/extensions/airdrop";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  airdropClaimedEvent()
 * ],
 * });
 * ```
 */
export declare function airdropClaimedEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "AirdropClaimed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "token";
    }, {
        readonly type: "address";
        readonly name: "receiver";
    }];
}>;
//# sourceMappingURL=AirdropClaimed.d.ts.map