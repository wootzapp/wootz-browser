"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.ThemedView = ThemedView;
const jsx_runtime_1 = require("react/jsx-runtime");
const react_native_1 = require("react-native");
function ThemedView({ style, theme, ...otherProps }) {
    return ((0, jsx_runtime_1.jsx)(react_native_1.View, { style: [{ backgroundColor: theme.colors.modalBg }, style], ...otherProps }));
}
//# sourceMappingURL=view.js.map