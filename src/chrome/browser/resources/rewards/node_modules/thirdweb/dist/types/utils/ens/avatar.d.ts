import type { ThirdwebClient } from "../../client/client.js";
export type ParseAvatarOptions = {
    client: ThirdwebClient;
    uri: string;
};
/**
 * Parses an ENS or similar avatar record. Supports NFT URIs, IPFS scheme, and HTTPS URIs.
 * @param options - The options for parsing an ENS avatar record.
 * @param options.client - The Thirdweb client.
 * @param options.uri - The URI to parse.
 * @returns A promise that resolves to the avatar URL, or null if the URI could not be parsed.
 * @example
 * ```ts
 * import { parseAvatarRecord } from "thirdweb/utils/ens";
 * const avatarUrl = await parseAvatarRecord({
 *    client,
 *    uri: "ipfs://bafybeiemxf5abjwjbikoz4mc3a3dla6ual3jsgpdr4cjr3oz3evfyavhwq/",
 * });
 *
 * console.log(avatarUrl); // "https://ipfs.io/ipfs/bafybeiemxf5abjwjbikoz4mc3a3dla6ual3jsgpdr4cjr3oz3evfyavhwq/"
 *
 * const avatarUrl2 = await parseAvatarRecord({
 *    client,
 *    uri: "eip155:1/erc1155:0xb32979486938aa9694bfc898f35dbed459f44424/10063",
 * });
 *
 * console.log(avatarUrl2); // "https://opensea.io/assets/0xb32979486938aa9694bfc898f35dbed459f44424/10063"
 * ```
 * @extension ENS
 */
export declare function parseAvatarRecord(options: ParseAvatarOptions): Promise<string | null>;
//# sourceMappingURL=avatar.d.ts.map