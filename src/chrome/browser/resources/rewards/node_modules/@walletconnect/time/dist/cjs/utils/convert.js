"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.fromMiliseconds = exports.toMiliseconds = void 0;
const constants_1 = require("../constants");
function toMiliseconds(seconds) {
    return seconds * constants_1.ONE_THOUSAND;
}
exports.toMiliseconds = toMiliseconds;
function fromMiliseconds(miliseconds) {
    return Math.floor(miliseconds / constants_1.ONE_THOUSAND);
}
exports.fromMiliseconds = fromMiliseconds;
//# sourceMappingURL=convert.js.map