import { type Abi } from "abitype";
import { type Hex } from "../../../utils/encoding/hex.js";
import type { ClientAndChainAndAccount } from "../../../utils/types.js";
/**
 * @internal
 */
export declare function zkDeployContractDeterministic(options: ClientAndChainAndAccount & {
    abi: Abi;
    bytecode: Hex;
    params?: Record<string, unknown>;
    salt?: string;
}): Promise<string>;
//# sourceMappingURL=zkDeployDeterministic.d.ts.map