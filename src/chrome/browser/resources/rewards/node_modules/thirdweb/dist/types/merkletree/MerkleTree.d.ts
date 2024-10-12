import { type Hex } from "../utils/encoding/hex.js";
/**
 * Class reprensenting a Merkle Tree
 * @namespace MerkleTree
 */
export declare class MerkleTree {
    private leaves;
    private layers;
    constructor(leaves: (Uint8Array | Hex)[]);
    getHexRoot(): `0x${string}`;
    getHexProof(leaf: Uint8Array | Hex, index?: number): `0x${string}`[];
    private createHashes;
    private getRoot;
    private getProof;
}
//# sourceMappingURL=MerkleTree.d.ts.map