"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.decodeJWT = decodeJWT;
const uint8_array_js_1 = require("../uint8-array.js");
const jwt_header_js_1 = require("./jwt-header.js");
/**
 * Decodes a JSON Web Token (JWT) and returns the decoded payload and signature.
 * @param jwt - The JWT string to decode.
 * @returns An object containing the decoded payload and signature.
 * @throws {Error} If the JWT header is invalid or if the JWT is invalid.
 * @example
 * ```ts
 * import { decodeJWT } from 'thirdweb/utils';
 *
 * const { payload, signature } = decodeJWT(jwt);
 * ```
 * @auth
 */
function decodeJWT(jwt) {
    const [encodedHeader, encodedPayload, encodedSignature] = jwt.split(".");
    if (encodedHeader !== jwt_header_js_1.PRECOMPILED_B64_ENCODED_JWT_HEADER) {
        throw new Error("Invalid JWT header");
    }
    if (!encodedPayload || !encodedSignature) {
        throw new Error("Invalid JWT");
    }
    const payload = JSON.parse((0, uint8_array_js_1.base64ToString)(encodedPayload));
    const signature = (0, uint8_array_js_1.base64ToString)(encodedSignature);
    return {
        payload,
        signature,
    };
}
//# sourceMappingURL=decode-jwt.js.map