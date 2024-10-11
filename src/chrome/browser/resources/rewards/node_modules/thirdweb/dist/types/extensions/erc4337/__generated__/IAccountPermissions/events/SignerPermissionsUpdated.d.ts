import type { AbiParameterToPrimitiveType } from "abitype";
/**
 * Represents the filters for the "SignerPermissionsUpdated" event.
 */
export type SignerPermissionsUpdatedEventFilters = Partial<{
    authorizingSigner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "authorizingSigner";
        indexed: true;
    }>;
    targetSigner: AbiParameterToPrimitiveType<{
        type: "address";
        name: "targetSigner";
        indexed: true;
    }>;
}>;
/**
 * Creates an event object for the SignerPermissionsUpdated event.
 * @param filters - Optional filters to apply to the event.
 * @returns The prepared event object.
 * @extension ERC4337
 * @example
 * ```ts
 * import { getContractEvents } from "thirdweb";
 * import { signerPermissionsUpdatedEvent } from "thirdweb/extensions/erc4337";
 *
 * const events = await getContractEvents({
 * contract,
 * events: [
 *  signerPermissionsUpdatedEvent({
 *  authorizingSigner: ...,
 *  targetSigner: ...,
 * })
 * ],
 * });
 * ```
 */
export declare function signerPermissionsUpdatedEvent(filters?: SignerPermissionsUpdatedEventFilters): import("../../../../../event/prepare-event.js").PreparedEvent<{
    readonly name: "SignerPermissionsUpdated";
    readonly type: "event";
    readonly inputs: readonly [{
        readonly type: "address";
        readonly name: "authorizingSigner";
        readonly indexed: true;
    }, {
        readonly type: "address";
        readonly name: "targetSigner";
        readonly indexed: true;
    }, {
        readonly type: "tuple";
        readonly components: readonly [{
            readonly type: "address";
            readonly name: "signer";
        }, {
            readonly type: "uint8";
            readonly name: "isAdmin";
        }, {
            readonly type: "address[]";
            readonly name: "approvedTargets";
        }, {
            readonly type: "uint256";
            readonly name: "nativeTokenLimitPerTransaction";
        }, {
            readonly type: "uint128";
            readonly name: "permissionStartTimestamp";
        }, {
            readonly type: "uint128";
            readonly name: "permissionEndTimestamp";
        }, {
            readonly type: "uint128";
            readonly name: "reqValidityStartTimestamp";
        }, {
            readonly type: "uint128";
            readonly name: "reqValidityEndTimestamp";
        }, {
            readonly type: "bytes32";
            readonly name: "uid";
        }];
        readonly name: "permissions";
    }];
}>;
//# sourceMappingURL=SignerPermissionsUpdated.d.ts.map