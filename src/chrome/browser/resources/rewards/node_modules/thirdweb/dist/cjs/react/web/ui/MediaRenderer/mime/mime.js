"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getMimeTypeFromUrl = getMimeTypeFromUrl;
const types_js_1 = require("./types.js");
/**
 * Get mime type from a URL
 * @internal
 */
function getMimeTypeFromUrl(url) {
    const last = url.replace(/^.*[/\\]/, "").toLowerCase();
    const fileExtension = last.replace(/^.*\./, "").toLowerCase();
    const hasPath = last.length < url.length;
    const hasDot = fileExtension.length < last.length - 1;
    return ((hasDot || !hasPath) && types_js_1.extensionsToMimeType[fileExtension]) || null;
}
//# sourceMappingURL=mime.js.map