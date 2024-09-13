import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "PoolCreated" event.
 */
export type PoolCreatedEventFilters = Partial<{
    token0: AbiParameterToPrimitiveType<{
        type: "address";
        name: "token0";
        indexed: true;
    }>;
    token1: AbiParameterToPrimitiveType<{
        type: "address";
        name: "token1";
        indexed: true;
    }>;
    sender: AbiParameterToPrimitiveType<{
        type: "address";
        name: "sender";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the PoolCreated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension UNISWAP
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { poolCreatedEvent } from "thirdweb/extensions/uniswap";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  poolCreatedEvent({
 *  token0: ...,
 *  token1: ...,
 *  sender: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function poolCreatedEvent(filters?: PoolCreatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "PoolCreated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "token0";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "token1";
        readonly indexed: true;
    }, {
        readonly type: "uint24";
        readonly name: "fee";
    }, {
        readonly type: "int24";
        readonly name: "tickLower";
    }, {
        readonly type: "int24";
        readonly name: "tickUpper";
    }, {
        readonly type: "uint128";
        readonly name: "liquidity";
    }, {
        readonly type: "address";
        readonly name: "sender";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=PoolCreated.d.ts.map