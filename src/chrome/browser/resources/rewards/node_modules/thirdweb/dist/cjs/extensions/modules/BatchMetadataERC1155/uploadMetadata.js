"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.uploadMetadata = uploadMetadata;
const uploadMetadata_js_1 = require("../BatchMetadataERC721/uploadMetadata.js");
/**
 * Uploads metadata for a batch of NFTs.
 * @param options - The options for the transaction.
 * @param options.contract - The contract to upload the metadata for.
 * @param options.metadatas - The metadata for the NFTs.
 * @returns The transaction to upload the metadata.
 * @modules BatchMetadataERC1155
 * @example
 * ```ts
 * import { BatchMetadataERC1155 } from "thirdweb/modules";
 *
 * const transaction = BatchMetadataERC1155.uploadMetadata({
 *   contract,
 *   metadatas: [
 *     { name: "My NFT", description: "This is my NFT" },
 *   ],
 * });
 *
 * await sendTransaction({
 *   transaction,
 *   account,
 * });
 * ```
 */
function uploadMetadata(options) {
    return (0, uploadMetadata_js_1.uploadMetadata)(options);
}
//# sourceMappingURL=uploadMetadata.js.map