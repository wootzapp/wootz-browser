// src/prefetch.ts
import { useQueryClient } from "./QueryClientProvider.js";
function usePrefetchQuery(options) {
  const queryClient = useQueryClient();
  if (!queryClient.getQueryState(options.queryKey)) {
    queryClient.prefetchQuery(options);
  }
}
function usePrefetchInfiniteQuery(options) {
  const queryClient = useQueryClient();
  if (!queryClient.getQueryState(options.queryKey)) {
    queryClient.prefetchInfiniteQuery(options);
  }
}
export {
  usePrefetchInfiniteQuery,
  usePrefetchQuery
};
//# sourceMappingURL=prefetch.js.map