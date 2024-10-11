"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.wait = void 0;
/**
 * @internal
 */
const wait = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
exports.wait = wait;
//# sourceMappingURL=wait.js.map