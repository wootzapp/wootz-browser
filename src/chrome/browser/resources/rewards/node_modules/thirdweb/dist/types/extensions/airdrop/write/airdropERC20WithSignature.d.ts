import type { AbiParameterToPrimitiveType, Address } from "abitype";
import type { ThirdwebContract } from "../../../contract/contract.js";
import type { Hex } from "../../../utils/encoding/hex.js";
import type { Account } from "../../../wallets/interfaces/wallet.js";
import { airdropERC20WithSignature as generatedAirdropERC20WithSignature } from "../__generated__/Airdrop/write/airdropERC20WithSignature.js";
/**
 * Airdrops ERC20 tokens to a list of recipients, with the request signed by admin
 * @param options - The transaction options.
 * @example
 * ```ts
 * import { airdropERC20WithSignature, generateAirdropSignatureERC20 } from "thirdweb/extensions/airdrop";
 * import { sendTransaction } from "thirdweb";
 *
 * const { req, signature } = await generateAirdropSignatureERC20(...)
 *
 * const transaction = airdropERC20WithSignature({
 *   contract,
 *   req,
 *   signature,
 * });
 * await sendTransaction({ transaction, account });
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the transaction result.
 */
export declare const airdropERC20WithSignature: typeof generatedAirdropERC20WithSignature;
/**
 * @extension AIRDROP
 */
export type GenerateAirdropERC20SignatureOptions = {
    account: Account;
    contract: ThirdwebContract;
    airdropRequest: GenerateReqInput;
};
/**
 * Generates the req and signature for sending ERC20 airdrop.
 * @param options - The options for the airdrop.
 * @example
 * ```ts
 * import { airdropERC20WithSignature, generateAirdropSignatureERC20 } from "thirdweb/extensions/airdrop";
 *
 * // list of recipients and amounts to airdrop for each recipient
 * const contents = [
 *    { recipient: "0x...", amount: 10n }, // amount in wei
 *    { recipient: "0x...", amount: 15n }, // amount in wei
 *    { recipient: "0x...", amount: 20n }, // amount in wei
 *  ];
 *
 * const { req, signature } = await generateAirdropSignatureERC20({
 *   account,
 *   contract,
 *   airdropRequest: {
 *     tokenAddress: "0x...", // address of the ERC20 token to airdrop
 *     contents
 *   },
 * });
 *
 * const transaction = airdropERC20WithSignature({
 *   contract,
 *   req,
 *   signature,
 * });
 * await sendTransaction({ transaction, account });
 * ```
 * @extension AIRDROP
 * @returns A promise that resolves to the req and signature.
 */
export declare function generateAirdropSignatureERC20(options: GenerateAirdropERC20SignatureOptions): Promise<{
    req: {
        uid: `0x${string}`;
        tokenAddress: string;
        expirationTimestamp: bigint;
        contents: readonly {
            recipient: string;
            amount: bigint;
        }[];
    };
    signature: `0x${string}`;
}>;
type ContentType = AbiParameterToPrimitiveType<{
    type: "tuple[]";
    name: "contents";
    components: typeof AirdropContentERC20;
}>;
type GenerateReqInput = {
    uid?: Hex;
    tokenAddress: Address;
    expirationTimestamp?: Date;
    contents: ContentType;
};
declare const AirdropContentERC20: readonly [{
    readonly name: "recipient";
    readonly type: "address";
}, {
    readonly name: "amount";
    readonly type: "uint256";
}];
export {};
//# sourceMappingURL=airdropERC20WithSignature.d.ts.map