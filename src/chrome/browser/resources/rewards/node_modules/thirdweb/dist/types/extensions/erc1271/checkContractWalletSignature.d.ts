import { type SignableMessage } from "viem";
import type { ThirdwebContract } from "../../contract/contract.js";
/**
 * @extension ERC1271
 */
export type CheckContractWalletSignatureOptions = {
    contract: ThirdwebContract;
    message: SignableMessage;
    signature: string;
};
/**
 * Checks if a contract wallet signature is valid.
 * @deprecated Use `verifySignature` instead
 * @param options - The options for the checkContractWalletSignature function.
 * @param options.contract - The contract to check the signature against.
 * @param options.message - The message to check the signature against.
 * @param options.signature - The signature to check.
 * @extension ERC1271
 * @example
 * ```ts
 * import { checkContractWalletSignature } from "thirdweb/extensions/erc1271";
 * const isValid = await checkContractWalletSignature({
 *  contract: myContract,
 *  message: "hello world",
 *  signature: "0x...",
 * });
 * ```
 * @returns A promise that resolves with a boolean indicating if the signature is valid.
 */
export declare function checkContractWalletSignature(options: CheckContractWalletSignatureOptions): Promise<boolean>;
//# sourceMappingURL=checkContractWalletSignature.d.ts.map