export type AddressInput = string;
export type Address = `0x${string}`;
/**
 * Checks if a given string is a valid address.
 * @param address The address to check.
 * @returns True if the address is valid, false otherwise.
 * @example
 * ```ts
 * import { isAddress } from 'thirdweb/utils';
 *
 * isAddress('0x5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed');
 * //=> true
 * ```
 * @utils
 */
export declare function isAddress(address: string): address is Address;
/**
 * Calculates the checksum address for the given address.
 * @param address - The address to calculate the checksum for.
 * @returns The checksum address.
 * @example
 * ```ts
 * import { checksumAddress } from 'thirdweb/utils';
 *
 * checksumAddress('0x5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed');
 * //=> '0x5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed'
 * ```
 * @utils
 */
export declare function checksumAddress(address: string): Address;
/**
 * Retrieves the address after performing validation and checksumming.
 * @param address - The address to be validated and checksummed.
 * @returns The validated and checksummed address.
 * @throws Error if the address is invalid.
 * @example
 * ```ts
 * import { getAddress } from 'thirdweb/utils';
 *
 * getAddress('0x5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed');
 * //=> '0x5aAeb6053F3E94C9b9A09f33669435E7Ef1BeAed'
 * ```
 * @utils
 */
export declare function getAddress(address: string): Address;
/**
 * Checksums and formats an address if valid. Note this function does not check if the provided address is an ENS.
 * @param address - The address to shorten.
 * @param length - The number of characters to keep from the start and end of the address.
 * @returns The shortened address.
 * @example
 * ```ts
 * import { shortenAddress } from 'thirdweb/utils';
 *
 * shortenAddress('0xa0cf798816d4b9b9866b5330eea46a18382f251e');
 * //=> '0xA0Cf...251e'
 * ```
 * @utils
 */
export declare function shortenAddress(address: string, length?: number): string;
/**
 * Shortens a hexadecimal string without performing any validation or checksumming.
 * @param hex - The hexadecimal string to shorten.
 * @param length - The number of characters to keep from the start and end of the string.
 * @returns The shortened hexadecimal string.
 * @example
 * ```ts
 * import { shortenHex } from 'thirdweb/utils';
 *
 * shortenHex('0xa0cf798816d4b9b9866b5330eea46a18382f251e');
 * //=> '0xa0cf...251e'
 * ```
 * @utils
 */
export declare function shortenHex(hex: string, length?: number): string;
//# sourceMappingURL=address.d.ts.map