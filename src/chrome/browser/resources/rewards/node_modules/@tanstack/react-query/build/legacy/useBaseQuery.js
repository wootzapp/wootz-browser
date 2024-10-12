"use client";

// src/useBaseQuery.ts
import * as React from "react";
import { notifyManager } from "@tanstack/query-core";
import { useQueryErrorResetBoundary } from "./QueryErrorResetBoundary.js";
import { useQueryClient } from "./QueryClientProvider.js";
import { useIsRestoring } from "./isRestoring.js";
import {
  ensurePreventErrorBoundaryRetry,
  getHasError,
  useClearResetErrorBoundary
} from "./errorBoundaryUtils.js";
import {
  ensureSuspenseTimers,
  fetchOptimistic,
  shouldSuspend
} from "./suspense.js";
function useBaseQuery(options, Observer, queryClient) {
  var _a, _b, _c, _d;
  if (process.env.NODE_ENV !== "production") {
    if (typeof options !== "object" || Array.isArray(options)) {
      throw new Error(
        'Bad argument type. Starting with v5, only the "Object" form is allowed when calling query related functions. Please use the error stack to find the culprit call. More info here: https://tanstack.com/query/latest/docs/react/guides/migrating-to-v5#supports-a-single-signature-one-object'
      );
    }
  }
  const client = useQueryClient(queryClient);
  const isRestoring = useIsRestoring();
  const errorResetBoundary = useQueryErrorResetBoundary();
  const defaultedOptions = client.defaultQueryOptions(options);
  (_b = (_a = client.getDefaultOptions().queries) == null ? void 0 : _a._experimental_beforeQuery) == null ? void 0 : _b.call(
    _a,
    defaultedOptions
  );
  defaultedOptions._optimisticResults = isRestoring ? "isRestoring" : "optimistic";
  ensureSuspenseTimers(defaultedOptions);
  ensurePreventErrorBoundaryRetry(defaultedOptions, errorResetBoundary);
  useClearResetErrorBoundary(errorResetBoundary);
  const [observer] = React.useState(
    () => new Observer(
      client,
      defaultedOptions
    )
  );
  const result = observer.getOptimisticResult(defaultedOptions);
  React.useSyncExternalStore(
    React.useCallback(
      (onStoreChange) => {
        const unsubscribe = isRestoring ? () => void 0 : observer.subscribe(notifyManager.batchCalls(onStoreChange));
        observer.updateResult();
        return unsubscribe;
      },
      [observer, isRestoring]
    ),
    () => observer.getCurrentResult(),
    () => observer.getCurrentResult()
  );
  React.useEffect(() => {
    observer.setOptions(defaultedOptions, { listeners: false });
  }, [defaultedOptions, observer]);
  if (shouldSuspend(defaultedOptions, result)) {
    throw fetchOptimistic(defaultedOptions, observer, errorResetBoundary);
  }
  if (getHasError({
    result,
    errorResetBoundary,
    throwOnError: defaultedOptions.throwOnError,
    query: client.getQueryCache().get(defaultedOptions.queryHash)
  })) {
    throw result.error;
  }
  ;
  (_d = (_c = client.getDefaultOptions().queries) == null ? void 0 : _c._experimental_afterQuery) == null ? void 0 : _d.call(
    _c,
    defaultedOptions,
    result
  );
  return !defaultedOptions.notifyOnChangeProps ? observer.trackResult(result) : result;
}
export {
  useBaseQuery
};
//# sourceMappingURL=useBaseQuery.js.map