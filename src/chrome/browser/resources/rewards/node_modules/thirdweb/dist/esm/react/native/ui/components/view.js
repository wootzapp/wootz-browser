import { jsx as _jsx } from "react/jsx-runtime";
import { View } from "react-native";
export function ThemedView({ style, theme, ...otherProps }) {
    return (_jsx(View, { style: [{ backgroundColor: theme.colors.modalBg }, style], ...otherProps }));
}
//# sourceMappingURL=view.js.map