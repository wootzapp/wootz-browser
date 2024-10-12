/**
 * Creates an event object for the ModuleInstalled event.
 * @returns The prepared event object.
 * @extension MODULES
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { moduleInstalledEvent } from "thirdweb/extensions/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  moduleInstalledEvent()
 * ],
 * });
 * ```
 */
export declare function moduleInstalledEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ModuleInstalled";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "caller";
    }, {
        readonly type: "address";
        readonly name: "implementation";
    }, {
        readonly type: "address";
        readonly name: "installedModule";
    }];
}>;
//# sourceMappingURL=ModuleInstalled.d.ts.map