import { DefaultError, QueryKey, FetchQueryOptions, FetchInfiniteQueryOptions } from '@tanstack/query-core';

declare function usePrefetchQuery<TQueryFnData = unknown, TError = DefaultError, TData = TQueryFnData, TQueryKey extends QueryKey = QueryKey>(options: FetchQueryOptions<TQueryFnData, TError, TData, TQueryKey>): void;
declare function usePrefetchInfiniteQuery<TQueryFnData = unknown, TError = DefaultError, TData = TQueryFnData, TQueryKey extends QueryKey = QueryKey, TPageParam = unknown>(options: FetchInfiniteQueryOptions<TQueryFnData, TError, TData, TQueryKey, TPageParam>): void;

export { usePrefetchInfiniteQuery, usePrefetchQuery };
