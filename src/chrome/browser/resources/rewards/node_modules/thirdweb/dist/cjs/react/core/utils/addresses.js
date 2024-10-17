"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.shortenString = shortenString;
/**
 * @internal
 */
function shortenString(str, extraShort = true) {
    return `${str.substring(0, extraShort ? 4 : 6)}...${str.substring(str.length - (extraShort ? 3 : 4))}`;
}
//# sourceMappingURL=addresses.js.map