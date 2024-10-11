import type { AbiParameterToPrimitiveType, Address } from "abitype";
import type { ThirdwebContract } from "../../../contract/contract.js";
import type { Hex } from "../../../utils/encoding/hex.js";
import type { Account } from "../../../wallets/interfaces/wallet.js";
import { airdropERC1155WithSignature as generatedAirdropERC1155WithSignature } from "../__generated__/Airdrop/write/airdropERC1155WithSignature.js";
/**
 * Airdrops ERC1155 tokens to a list of recipients, with the request signed by admin
 * @param options - The transaction options.
 * @example
 * ```ts
 * import { airdropERC1155WithSignature, generateAirdropSignatureERC1155 } from "thirdweb/extensions/airdrop";
 * import { sendTransaction } from "thirdweb";
 *
 * const { req, signature } = await generateAirdropSignatureERC1155(...)
 *
 * const transaction = airdropERC1155WithSignature({
 *   contract,
 *   req,
 *   signature,
 * });
 * await sendTransaction({ transaction, account });
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the transaction result.
 */
export declare const airdropERC1155WithSignature: typeof generatedAirdropERC1155WithSignature;
/**
 * @extension AIRDROP
 */
export type GenerateAirdropERC1155SignatureOptions = {
    account: Account;
    contract: ThirdwebContract;
    airdropRequest: GenerateReqInput;
};
/**
 * Generates the req and signature for sending ERC1155 airdrop.
 * @param options - The options for the airdrop.
 * @example
 * ```ts
 * import { airdropERC1155WithSignature, generateAirdropSignatureERC1155 } from "thirdweb/extensions/airdrop";
 *
 * // list of recipients, tokenIds and amounts to airdrop for each recipient
 * const contents = [
 *    { recipient: "0x...", tokenId: 0, amount: 10n },
 *    { recipient: "0x...", tokenId: 0, amount: 15n },
 *    { recipient: "0x...", tokenId: 0, amount: 20n },
 *  ];
 *
 * const { req, signature } = await generateAirdropSignatureERC1155({
 *   account,
 *   contract,
 *   airdropRequest: {
 *     tokenAddress: "0x...", // address of the ERC1155 token to airdrop
 *     contents
 *   },
 * });
 *
 * const transaction = airdropERC1155WithSignature({
 *   contract,
 *   req,
 *   signature,
 * });
 * await sendTransaction({ transaction, account });
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the req and signature.
 */
export declare function generateAirdropSignatureERC1155(options: GenerateAirdropERC1155SignatureOptions): Promise<{
    req: {
        uid: `0x${string}`;
        tokenAddress: string;
        expirationTimestamp: bigint;
        contents: readonly {
            recipient: string;
            tokenId: bigint;
            amount: bigint;
        }[];
    };
    signature: `0x${string}`;
}>;
type ContentType = AbiParameterToPrimitiveType<{
    type: "tuple[]";
    name: "contents";
    components: typeof AirdropContentERC1155;
}>;
type GenerateReqInput = {
    uid?: Hex;
    tokenAddress: Address;
    expirationTimestamp?: Date;
    contents: ContentType;
};
declare const AirdropContentERC1155: readonly [{
    readonly name: "recipient";
    readonly type: "address";
}, {
    readonly name: "tokenId";
    readonly type: "uint256";
}, {
    readonly name: "amount";
    readonly type: "uint256";
}];
export {};
//# sourceMappingURL=airdropERC1155WithSignature.d.ts.map