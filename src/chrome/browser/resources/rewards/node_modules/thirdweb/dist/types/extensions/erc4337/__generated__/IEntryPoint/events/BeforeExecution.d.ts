/**
 * Creates an event object for the BeforeExecution event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { beforeExecutionEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  beforeExecutionEvent()
 * ],
 * });
 * ```
 */
export declare function beforeExecutionEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "BeforeExecution";
    readonly type: "event";
    readonly inputs: readonly [];
}>;
//# sourceMappingURL=BeforeExecution.d.ts.map