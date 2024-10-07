import { useQueryClient } from "@tanstack/react-query";
/**
 * @internal
 */
export function useInvalidateContractQuery() {
    const queryClient = useQueryClient();
    return ({ chainId, contractAddress, }) => {
        queryClient.invalidateQueries({
            queryKey: ["readContract", chainId, contractAddress],
        });
    };
}
//# sourceMappingURL=useInvalidateQueries.js.map