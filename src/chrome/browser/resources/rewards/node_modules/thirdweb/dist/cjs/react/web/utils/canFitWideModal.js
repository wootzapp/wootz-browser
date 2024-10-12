"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.canFitWideModal = canFitWideModal;
const constants_js_1 = require("../ui/ConnectWallet/constants.js");
/**
 * Return true if the current screen size can fit the wide connect modal
 * @internal
 */
function canFitWideModal() {
    if (typeof window !== "undefined") {
        return window.innerWidth >= constants_js_1.wideModalScreenThreshold;
    }
    return false;
}
//# sourceMappingURL=canFitWideModal.js.map