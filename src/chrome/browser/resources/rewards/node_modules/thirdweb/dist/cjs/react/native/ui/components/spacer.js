"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Spacer = Spacer;
const jsx_runtime_1 = require("react/jsx-runtime");
const react_native_1 = require("react-native");
const index_js_1 = require("../../design-system/index.js");
function Spacer({ size }) {
    return (0, jsx_runtime_1.jsx)(react_native_1.View, { style: { height: index_js_1.spacing[size] } });
}
//# sourceMappingURL=spacer.js.map