import type { Hex } from "../../utils/encoding/hex.js";
/**
 * @extension FARCASTER
 */
export type Ed25519Keypair = {
    publicKey: Hex;
    privateKey: Hex;
};
/**
 * Generates an Ed25519 keypair to be used as an account signer.
 * @returns A promise resolving to the generated keypair.
 * @example
 * ```ts
 * createSigner()
 * ```
 * @extension FARCASTER
 */
export declare function createEd25519Keypair(): Promise<Ed25519Keypair>;
//# sourceMappingURL=ed25519.d.ts.map