import { checksumAddress } from "../../../utils/address.js";
/**
 * @internal
 */
export function validateAccountAddress(account, address) {
    if (checksumAddress(account.address) !== checksumAddress(address)) {
        throw new Error(`Failed to validate account address (${account.address}), differs from ${address}`);
    }
}
/**
 * @internal
 */
export function parseEip155ChainId(chainId) {
    const chainIdParts = chainId.split(":");
    const chainIdAsNumber = Number.parseInt(chainIdParts[1] ?? "0");
    if (chainIdParts.length !== 2 ||
        chainIdParts[0] !== "eip155" ||
        chainIdAsNumber === 0 ||
        !chainIdAsNumber) {
        throw new Error(`Invalid chainId ${chainId}, should have the format 'eip155:1'`);
    }
    return chainIdAsNumber;
}
//# sourceMappingURL=utils.js.map