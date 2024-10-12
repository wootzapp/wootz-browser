"use client";
import { jsx as _jsx } from "react/jsx-runtime";
import { createContext, useContext, useState } from "react";
import { CustomThemeProvider } from "../../core/design-system/CustomThemeProvider.js";
const WalletModalOpen = /* @__PURE__ */ createContext(false);
const SetWalletModalOpen = /* @__PURE__ */ createContext(() => { });
const SelectionUIDataCtx = /* @__PURE__ */ createContext({});
const SetSelectionUIDataCtx = /* @__PURE__ */ createContext(() => { });
/**
 * @internal
 */
export const WalletUIStatesProvider = (props) => {
    const [isWalletModalOpen, setIsWalletModalOpen] = useState(props.isOpen);
    const [selectionUIData, setSelectionUIData] = useState({}); // allow any type of object
    return (_jsx(WalletModalOpen.Provider, { value: isWalletModalOpen, children: _jsx(SetWalletModalOpen.Provider, { value: setIsWalletModalOpen, children: _jsx(SelectionUIDataCtx.Provider, { value: selectionUIData, children: _jsx(SetSelectionUIDataCtx.Provider, { value: setSelectionUIData, children: _jsx(CustomThemeProvider, { theme: props.theme, children: props.children }) }) }) }) }));
};
/**
 * @internal
 */
export const useIsWalletModalOpen = () => {
    return useContext(WalletModalOpen);
};
/**
 * @internal
 */
export const useSetIsWalletModalOpen = () => {
    const context = useContext(SetWalletModalOpen);
    if (context === undefined) {
        throw new Error("useSetWalletModalOpen must be used within a ThirdwebProvider");
    }
    return context;
};
/**
 * @internal
 */
export function useSetSelectionData() {
    return useContext(SetSelectionUIDataCtx);
}
/**
 * @internal
 */
export function useSelectionData() {
    return useContext(SelectionUIDataCtx);
}
//# sourceMappingURL=wallet-ui-states-provider.js.map