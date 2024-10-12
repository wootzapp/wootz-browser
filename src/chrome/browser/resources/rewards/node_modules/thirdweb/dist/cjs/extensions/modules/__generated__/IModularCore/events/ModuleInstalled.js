"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.moduleInstalledEvent = moduleInstalledEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function moduleInstalledEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event ModuleInstalled(address caller, address implementation, address installedModule)",
    });
}
//# sourceMappingURL=ModuleInstalled.js.map