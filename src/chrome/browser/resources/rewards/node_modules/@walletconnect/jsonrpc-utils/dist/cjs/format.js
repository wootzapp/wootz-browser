"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.formatErrorMessage = exports.formatJsonRpcError = exports.formatJsonRpcResult = exports.formatJsonRpcRequest = exports.getBigIntRpcId = exports.payloadId = void 0;
const error_1 = require("./error");
const constants_1 = require("./constants");
function payloadId(entropy = 3) {
    const date = Date.now() * Math.pow(10, entropy);
    const extra = Math.floor(Math.random() * Math.pow(10, entropy));
    return date + extra;
}
exports.payloadId = payloadId;
function getBigIntRpcId(entropy = 6) {
    return BigInt(payloadId(entropy));
}
exports.getBigIntRpcId = getBigIntRpcId;
function formatJsonRpcRequest(method, params, id) {
    return {
        id: id || payloadId(),
        jsonrpc: "2.0",
        method,
        params,
    };
}
exports.formatJsonRpcRequest = formatJsonRpcRequest;
function formatJsonRpcResult(id, result) {
    return {
        id,
        jsonrpc: "2.0",
        result,
    };
}
exports.formatJsonRpcResult = formatJsonRpcResult;
function formatJsonRpcError(id, error, data) {
    return {
        id,
        jsonrpc: "2.0",
        error: formatErrorMessage(error, data),
    };
}
exports.formatJsonRpcError = formatJsonRpcError;
function formatErrorMessage(error, data) {
    if (typeof error === "undefined") {
        return (0, error_1.getError)(constants_1.INTERNAL_ERROR);
    }
    if (typeof error === "string") {
        error = Object.assign(Object.assign({}, (0, error_1.getError)(constants_1.SERVER_ERROR)), { message: error });
    }
    if (typeof data !== "undefined") {
        error.data = data;
    }
    if ((0, error_1.isReservedErrorCode)(error.code)) {
        error = (0, error_1.getErrorByCode)(error.code);
    }
    return error;
}
exports.formatErrorMessage = formatErrorMessage;
//# sourceMappingURL=format.js.map