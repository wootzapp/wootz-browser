"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isAbiEvent = isAbiEvent;
/**
 * @internal
 */
function isAbiEvent(item) {
    return !!(item &&
        typeof item === "object" &&
        "type" in item &&
        item.type === "event");
}
//# sourceMappingURL=utils.js.map