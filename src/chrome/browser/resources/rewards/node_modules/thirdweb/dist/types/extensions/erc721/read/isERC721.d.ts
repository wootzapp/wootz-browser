import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Check if a contract supports the ERC721 interface.
 * @param options - The transaction options.
 * @returns A boolean indicating whether the contract supports the ERC721 interface.
 * @extension ERC721
 * @example
 * ```ts
 * import { isERC721 } from "thirdweb/extensions/erc721";
 * const result = await isERC721({ contract });
 * ```
 */
export declare function isERC721(options: BaseTransactionOptions): Promise<boolean>;
//# sourceMappingURL=isERC721.d.ts.map