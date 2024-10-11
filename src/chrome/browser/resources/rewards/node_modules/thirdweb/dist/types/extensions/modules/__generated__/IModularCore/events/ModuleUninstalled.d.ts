/**
 * Creates an event object for the ModuleUninstalled event.
 * @returns The prepared event object.
 * @extension MODULES
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { moduleUninstalledEvent } from "thirdweb/extensions/modules";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  moduleUninstalledEvent()
 * ],
 * });
 * ```
 */
export declare function moduleUninstalledEvent(): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ModuleUninstalled";
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
//# sourceMappingURL=ModuleUninstalled.d.ts.map