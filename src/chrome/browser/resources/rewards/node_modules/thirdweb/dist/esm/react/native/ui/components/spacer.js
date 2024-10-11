import { jsx as _jsx } from "react/jsx-runtime";
import { View } from "react-native";
import { spacing } from "../../design-system/index.js";
export function Spacer({ size }) {
    return _jsx(View, { style: { height: spacing[size] } });
}
//# sourceMappingURL=spacer.js.map