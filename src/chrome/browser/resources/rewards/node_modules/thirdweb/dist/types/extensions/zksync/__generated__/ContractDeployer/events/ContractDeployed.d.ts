import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "ContractDeployed" event.
 */
export type ContractDeployedEventFilters = Partial<{
    deployerAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "deployerAddress";
        indexed: true;
    }>;
    bytecodeHash: AbiParameterToPrimitiveType<{
        type: "bytes32";
        name: "bytecodeHash";
        indexed: true;
    }>;
    contractAddress: AbiParameterToPrimitiveType<{
        type: "address";
        name: "contractAddress";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the ContractDeployed event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ZKSYNC
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { contractDeployedEvent } from "thirdweb/extensions/zksync";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  contractDeployedEvent({
 *  deployerAddress: ...,
 *  bytecodeHash: ...,
 *  contractAddress: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function contractDeployedEvent(filters?: ContractDeployedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "ContractDeployed";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "deployerAddress";
        readonly indexed: true;
    }, {
        readonly type: "bytes32";
        readonly name: "bytecodeHash";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "contractAddress";
        readonly indexed: true;
    }];
}>;
//# sourceMappingURL=ContractDeployed.d.ts.map