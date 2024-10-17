"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.uploadMobile = uploadMobile;
const mobile_js_1 = require("./upload/mobile.js");
/**
 * Batch upload arbitrary file or JSON data using the configured decentralized storage system.
 * Automatically uploads any file data within JSON objects and replaces them with hashes.
 * @param options - Options to pass through to the storage uploader class
 * @returns  The URIs of the uploaded data
 * @example
 * ```jsx
 * // Upload an image
 * launchImageLibrary({mediaType: 'photo'}, async response => {
 *   if (response.assets?.[0]) {
 *      const {fileName, type, uri} = response.assets[0];
 *      if (!uri) {
 *        throw new Error('No uri');
 *      }
 *      const resp = await uploadMobile({
 *        uri,
 *        type,
 *        name: fileName,
 *      });
 *    }
 *  });
 *
 * // Upload an array of JSON objects
 * const objects = [
 *  { name: "JSON 1", text: "Hello World" },
 *  { name: "JSON 2", trait: "Awesome" },
 * ];
 * const jsonUris = await uploadMobile(objects);
 * ```
 * @storage
 */
async function uploadMobile(options) {
    if (!options) {
        return [];
    }
    const data = options.files.filter((item) => item !== undefined);
    if (!data?.length) {
        return [];
    }
    return await (0, mobile_js_1.uploadBatchMobile)(options.client, data, options);
}
//# sourceMappingURL=uploadMobile.js.map