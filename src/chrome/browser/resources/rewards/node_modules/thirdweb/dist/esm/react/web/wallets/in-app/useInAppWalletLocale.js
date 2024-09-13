import { useEffect, useState } from "react";
import { getInAppWalletLocale } from "../shared/locale/getConnectLocale.js";
/**
 * @internal
 */
export function useInAppWalletLocale(localeId) {
    const [locale, setLocale] = useState(undefined);
    // TODO: move this to a useQuery hook
    // or at the very least dedupe it?
    useEffect(() => {
        getInAppWalletLocale(localeId).then((l) => {
            setLocale(l);
        });
    }, [localeId]);
    return locale;
}
//# sourceMappingURL=useInAppWalletLocale.js.map