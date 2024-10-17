import type { WalletInfo } from "../wallet-info.js";
import type { WalletId } from "../wallet-types.js";
/**
 * Retrieves the wallet based on the provided ID.
 * @param id - The ID of the wallet.
 * @returns A promise that resolves to the wallet.
 * @internal
 */
export declare function getWalletInfo<TImage extends boolean>(id: WalletId, image?: TImage): Promise<[TImage] extends [true] ? string : WalletInfo>;
//# sourceMappingURL=getWalletInfo.d.ts.map