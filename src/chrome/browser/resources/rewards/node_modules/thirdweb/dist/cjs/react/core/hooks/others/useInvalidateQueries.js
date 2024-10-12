"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.useInvalidateContractQuery = useInvalidateContractQuery;
const react_query_1 = require("@tanstack/react-query");
/**
 * @internal
 */
function useInvalidateContractQuery() {
    const queryClient = (0, react_query_1.useQueryClient)();
    return ({ chainId, contractAddress, }) => {
        queryClient.invalidateQueries({
            queryKey: ["readContract", chainId, contractAddress],
        });
    };
}
//# sourceMappingURL=useInvalidateQueries.js.map