"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.beforeExecutionEvent = beforeExecutionEvent;
const prepare_event_js_1 = require("../../../../../event/prepare-event.js");
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
function beforeExecutionEvent() {
    return (0, prepare_event_js_1.prepareEvent)({
        signature: "event BeforeExecution()",
    });
}
//# sourceMappingURL=BeforeExecution.js.map