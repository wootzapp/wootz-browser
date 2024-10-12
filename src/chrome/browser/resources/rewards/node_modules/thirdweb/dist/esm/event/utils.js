/**
 * @internal
 */
export function isAbiEvent(item) {
    return !!(item &&
        typeof item === "object" &&
        "type" in item &&
        item.type === "event");
}
//# sourceMappingURL=utils.js.map