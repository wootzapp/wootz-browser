import { type Address } from "../../address.js";
import type { Hex } from "../../encoding/hex.js";
export declare function create2Address(options: {
    sender: Address;
    bytecodeHash: Hex;
    salt: Hex;
    input?: Hex;
}): string;
//# sourceMappingURL=create2Address.d.ts.map