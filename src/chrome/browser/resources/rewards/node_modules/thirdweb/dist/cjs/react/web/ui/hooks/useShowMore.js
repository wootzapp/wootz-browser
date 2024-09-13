"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useShowMore = useShowMore;
const react_1 = require("react");
/**
 *
 * @internal
 */
function useShowMore(initialItemsToShow, itemsToAdd) {
    // start with showing first `initialItemsToShow` items, when the last item is in view, show `itemsToAdd` more
    const [itemsToShow, setItemsToShow] = (0, react_1.useState)(initialItemsToShow);
    const lastItemRef = (0, react_1.useCallback)((node) => {
        if (!node) {
            return;
        }
        const observer = new IntersectionObserver((entries) => {
            if (entries[0]?.isIntersecting) {
                setItemsToShow((prev) => prev + itemsToAdd); // show 10 more items
            }
        }, { threshold: 1 });
        observer.observe(node);
        // when the node is removed from the DOM, observer will be disconnected automatically by the browser
    }, [itemsToAdd]);
    return { itemsToShow, lastItemRef };
}
//# sourceMappingURL=useShowMore.js.map