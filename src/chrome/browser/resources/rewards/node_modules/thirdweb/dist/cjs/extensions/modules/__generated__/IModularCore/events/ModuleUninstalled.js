"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.moduleUninstalledEvent = moduleUninstalledEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function moduleUninstalledEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event ModuleUninstalled(address caller, address implementation, address installedModule)",
    });
}
//# sourceMappingURL=ModuleUninstalled.js.map