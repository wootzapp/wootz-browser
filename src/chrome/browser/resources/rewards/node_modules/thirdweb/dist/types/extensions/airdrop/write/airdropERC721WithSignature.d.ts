import type { AbiParameterToPrimitiveType, Address } from "abitype";
import type { ThirdwebContract } from "../../../contract/contract.js";
import type { Hex } from "../../../utils/encoding/hex.js";
import type { Account } from "../../../wallets/interfaces/wallet.js";
import { airdropERC721WithSignature as generatedAirdropERC721WithSignature } from "../__generated__/Airdrop/write/airdropERC721WithSignature.js";
/**
 * Airdrops ERC721 tokens to a list of recipients, with the request signed by admin
 * @param options - The transaction options.
 * @example
 * ```ts
 * import { airdropERC721WithSignature, generateAirdropSignatureERC721 } from "thirdweb/extensions/airdrop";
 * import { sendTransaction } from "thirdweb";
 *
 * const { req, signature } = await generateAirdropSignatureERC721(...)
 *
 * const transaction = airdropERC721WithSignature({
 *   contract,
 *   req,
 *   signature,
 * });
 * await sendTransaction({ transaction, account });
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the transaction result.
 */
export declare const airdropERC721WithSignature: typeof generatedAirdropERC721WithSignature;
/**
 * @extension AIRDROP
 */
export type GenerateAirdropERC721SignatureOptions = {
    account: Account;
    contract: ThirdwebContract;
    airdropRequest: GenerateReqInput;
};
/**
 * Generates the req and signature for sending ERC721 airdrop.
 * @param options - The options for the airdrop.
 * @example
 * ```ts
 * import { airdropERC721WithSignature, generateAirdropSignatureERC721 } from "thirdweb/extensions/airdrop";
 *
 * // list of recipients and tokenIds to airdrop for each recipient
 * const contents = [
 *    { recipient: "0x...", tokenId: 0 },
 *    { recipient: "0x...", tokenId: 1 },
 *    { recipient: "0x...", tokenId: 2 },
 *  ];
 *
 * const { req, signature } = await generateAirdropSignatureERC721({
 *   account,
 *   contract,
 *   airdropRequest: {
 *     tokenAddress: "0x...", // address of the ERC721 token to airdrop
 *     contents
 *   },
 * });
 *
 * const transaction = airdropERC721WithSignature({
 *   contract,
 *   req,
 *   signature,
 * });
 * await sendTransaction({ transaction, account });
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the req and signature.
 */
export declare function generateAirdropSignatureERC721(options: GenerateAirdropERC721SignatureOptions): Promise<{
    req: {
        uid: `0x${string}`;
        tokenAddress: string;
        expirationTimestamp: bigint;
        contents: readonly {
            recipient: string;
            tokenId: bigint;
        }[];
    };
    signature: `0x${string}`;
}>;
type ContentType = AbiParameterToPrimitiveType<{
    type: "tuple[]";
    name: "contents";
    components: typeof AirdropContentERC721;
}>;
type GenerateReqInput = {
    uid?: Hex;
    tokenAddress: Address;
    expirationTimestamp?: Date;
    contents: ContentType;
};
declare const AirdropContentERC721: readonly [{
    readonly name: "recipient";
    readonly type: "address";
}, {
    readonly name: "tokenId";
    readonly type: "uint256";
}];
export {};
//# sourceMappingURL=airdropERC721WithSignature.d.ts.map