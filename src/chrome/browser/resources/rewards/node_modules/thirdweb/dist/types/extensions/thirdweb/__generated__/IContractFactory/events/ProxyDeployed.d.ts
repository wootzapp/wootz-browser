import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ProxyDeployed" event.
 */
export type ProxyDeployedEventFilters = Partial<{
    implementation: AbiParameterToPrimitiveType<{
        type: "address";
        name: "implementation";
        indexed: true;
    }>;
    deployer: AbiParameterToPrimitiveType<{
        type: "address";
        name: "deployer";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ProxyDeployed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension THIRDWEB
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { proxyDeployedEvent } from "thirdweb/extensions/thirdweb";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  proxyDeployedEvent({
 *  implementation: ...,
 *  deployer: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function proxyDeployedEvent(filters?: ProxyDeployedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ProxyDeployed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "implementation";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "proxy";
    }, {
        readonly type: "address";
        readonly name: "deployer";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=ProxyDeployed.d.ts.map