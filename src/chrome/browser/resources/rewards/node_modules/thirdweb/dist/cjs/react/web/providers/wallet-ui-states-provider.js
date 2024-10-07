"use strict";
"use client";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useSetIsWalletModalOpen = exports.useIsWalletModalOpen = exports.WalletUIStatesProvider = void 0;
exports.useSetSelectionData = useSetSelectionData;
exports.useSelectionData = useSelectionData;
const jsx_runtime_1 = require("react/jsx-runtime");
const react_1 = require("react");
const CustomThemeProvider_js_1 = require("../../core/design-system/CustomThemeProvider.js");
const WalletModalOpen = /* @__PURE__ */ (0, react_1.createContext)(false);
const SetWalletModalOpen = /* @__PURE__ */ (0, react_1.createContext)(() => { });
const SelectionUIDataCtx = /* @__PURE__ */ (0, react_1.createContext)({});
const SetSelectionUIDataCtx = /* @__PURE__ */ (0, react_1.createContext)(() => { });
/**
 * @internal
 */
const WalletUIStatesProvider = (props) => {
    const [isWalletModalOpen, setIsWalletModalOpen] = (0, react_1.useState)(props.isOpen);
    const [selectionUIData, setSelectionUIData] = (0, react_1.useState)({}); // allow any type of object
    return ((0, jsx_runtime_1.jsx)(WalletModalOpen.Provider, { value: isWalletModalOpen, children: (0, jsx_runtime_1.jsx)(SetWalletModalOpen.Provider, { value: setIsWalletModalOpen, children: (0, jsx_runtime_1.jsx)(SelectionUIDataCtx.Provider, { value: selectionUIData, children: (0, jsx_runtime_1.jsx)(SetSelectionUIDataCtx.Provider, { value: setSelectionUIData, children: (0, jsx_runtime_1.jsx)(CustomThemeProvider_js_1.CustomThemeProvider, { theme: props.theme, children: props.children }) }) }) }) }));
};
exports.WalletUIStatesProvider = WalletUIStatesProvider;
/**
 * @internal
 */
const useIsWalletModalOpen = () => {
    return (0, react_1.useContext)(WalletModalOpen);
};
exports.useIsWalletModalOpen = useIsWalletModalOpen;
/**
 * @internal
 */
const useSetIsWalletModalOpen = () => {
    const context = (0, react_1.useContext)(SetWalletModalOpen);
    if (context === undefined) {
        throw new Error("useSetWalletModalOpen must be used within a ThirdwebProvider");
    }
    return context;
};
exports.useSetIsWalletModalOpen = useSetIsWalletModalOpen;
/**
 * @internal
 */
function useSetSelectionData() {
    return (0, react_1.useContext)(SetSelectionUIDataCtx);
}
/**
 * @internal
 */
function useSelectionData() {
    return (0, react_1.useContext)(SelectionUIDataCtx);
}
//# sourceMappingURL=wallet-ui-states-provider.js.map