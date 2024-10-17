import type { BaseTransactionOptions } from "../../../transaction/types.js";
/**
 * Get the addresses of all recipients of a [`thirdweb Split contract`](https://thirdweb.com/thirdweb.eth/Split)
 * @extension SPLIT
 * @returns an array of wallet addresses
 *
 * @example
 * ```ts
 * import { getAllRecipientsAddresses } from "thirdweb/extensions/split";
 *
 * const addresses = await getAllRecipientsAddresses({ contract });
 */
export declare function getAllRecipientsAddresses(options: BaseTransactionOptions): Promise<string[]>;
//# sourceMappingURL=getAllRecipientsAddresses.d.ts.map