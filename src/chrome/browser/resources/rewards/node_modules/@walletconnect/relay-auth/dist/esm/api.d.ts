import * as ed25519 from "@stablelib/ed25519";
export declare function generateKeyPair(seed?: Uint8Array): ed25519.KeyPair;
export declare function signJWT(sub: string, aud: string, ttl: number, keyPair: ed25519.KeyPair, iat?: number): Promise<string>;
export declare function verifyJWT(jwt: string): Promise<boolean>;
//# sourceMappingURL=api.d.ts.map