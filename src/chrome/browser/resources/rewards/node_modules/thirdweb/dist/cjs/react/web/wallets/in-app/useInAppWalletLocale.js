"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useInAppWalletLocale = useInAppWalletLocale;
const react_1 = require("react");
const getConnectLocale_js_1 = require("../shared/locale/getConnectLocale.js");
/**
 * @internal
 */
function useInAppWalletLocale(localeId) {
    const [locale, setLocale] = (0, react_1.useState)(undefined);
    // TODO: move this to a useQuery hook
    // or at the very least dedupe it?
    (0, react_1.useEffect)(() => {
        (0, getConnectLocale_js_1.getInAppWalletLocale)(localeId).then((l) => {
            setLocale(l);
        });
    }, [localeId]);
    return locale;
}
//# sourceMappingURL=useInAppWalletLocale.js.map