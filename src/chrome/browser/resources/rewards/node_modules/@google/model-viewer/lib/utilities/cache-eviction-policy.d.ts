/**
 * A mutable cache is any object that has that allows cache
 * items to be deleted imperatively given their key
 */
export interface MutableCache<T> {
    delete(key: T): void;
}
declare const $retainerCount: unique symbol;
declare const $recentlyUsed: unique symbol;
declare const $evict: unique symbol;
declare const $evictionThreshold: unique symbol;
declare const $cache: unique symbol;
/**
 * The CacheEvictionPolicy manages the lifecycle for items in a cache,
 * evicting any items outside some threshold bounds in "recently used" order,
 * if they are evictable.
 *
 * Items are considered cached as they are retained. When all retainers
 * of an item release it, that item is considered evictable.
 */
export declare class CacheEvictionPolicy<T = string> {
    private [$retainerCount];
    private [$recentlyUsed];
    private [$evictionThreshold];
    private [$cache];
    constructor(cache: MutableCache<T>, evictionThreshold?: number);
    /**
     * The eviction threshold is the maximum number of items to hold
     * in cache indefinitely. Items within the threshold (in recently
     * used order) will continue to be cached even if they have zero
     * retainers.
     */
    set evictionThreshold(value: number);
    get evictionThreshold(): number;
    /**
     * A reference to the cache that operates under this policy
     */
    get cache(): MutableCache<T>;
    /**
     * Given an item key, returns the number of retainers of that item
     */
    retainerCount(key: T): number;
    /**
     * Resets the internal tracking of cache item retainers. Use only in cases
     * where it is certain that all retained cache items have been accounted for!
     */
    reset(): void;
    /**
     * Mark a given cache item as retained, where the item is represented
     * by its key. An item can have any number of retainers.
     */
    retain(key: T): void;
    /**
     * Mark a given cache item as released by one of its retainers, where the item
     * is represented by its key. When all retainers of an item have released it,
     * the item is considered evictable.
     */
    release(key: T): void;
    [$evict](): void;
}
export {};
