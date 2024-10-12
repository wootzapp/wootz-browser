export function once(fn) {
    let result;
    return () => {
        if (!result) {
            result = fn();
        }
        return result;
    };
}
//# sourceMappingURL=once.js.map