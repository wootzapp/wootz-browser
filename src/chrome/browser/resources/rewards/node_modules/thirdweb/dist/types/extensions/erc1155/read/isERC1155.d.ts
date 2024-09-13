import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Check if a contract supports the ERC1155 interface.
 * @param options - The transaction options.
 * @returns A boolean indicating whether the contract supports the ERC1155 interface.
 * @extension ERC1155
 * @example
 * ```ts
 * import { isERC1155 } from "thirdweb/extensions/erc1155";
 * const result = await isERC1155({ contract });
 * ```
 */
export declare function isERC1155(options: BaseTransactionOptions): Promise<boolean>;
//# sourceMappingURL=isERC1155.d.ts.map