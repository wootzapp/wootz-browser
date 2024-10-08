"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.PRECOMPILED_B64_ENCODED_JWT_HEADER = void 0;
/**
 * This is a precompile of the header for the JWT generated via:
 * ```ts
 * uint8ArrayToBase64(
 * stringToBytes(JSON.stringify(RAW_HEADER)),
 * )
 * ```
 */
exports.PRECOMPILED_B64_ENCODED_JWT_HEADER = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9";
//# sourceMappingURL=jwt-header.js.map