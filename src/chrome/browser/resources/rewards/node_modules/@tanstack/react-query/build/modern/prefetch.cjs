"use strict";
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// src/prefetch.ts
var prefetch_exports = {};
__export(prefetch_exports, {
  usePrefetchInfiniteQuery: () => usePrefetchInfiniteQuery,
  usePrefetchQuery: () => usePrefetchQuery
});
module.exports = __toCommonJS(prefetch_exports);
var import_QueryClientProvider = require("./QueryClientProvider.cjs");
function usePrefetchQuery(options) {
  const queryClient = (0, import_QueryClientProvider.useQueryClient)();
  if (!queryClient.getQueryState(options.queryKey)) {
    queryClient.prefetchQuery(options);
  }
}
function usePrefetchInfiniteQuery(options) {
  const queryClient = (0, import_QueryClientProvider.useQueryClient)();
  if (!queryClient.getQueryState(options.queryKey)) {
    queryClient.prefetchInfiniteQuery(options);
  }
}
// Annotate the CommonJS export names for ESM import in node:
0 && (module.exports = {
  usePrefetchInfiniteQuery,
  usePrefetchQuery
});
//# sourceMappingURL=prefetch.cjs.map