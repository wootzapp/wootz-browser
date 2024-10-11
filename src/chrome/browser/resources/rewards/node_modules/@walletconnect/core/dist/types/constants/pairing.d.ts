import { RelayerTypes, PairingJsonRpcTypes } from "@walletconnect/types";
export declare const PAIRING_CONTEXT = "pairing";
export declare const PAIRING_STORAGE_VERSION = "0.3";
export declare const PAIRING_DEFAULT_TTL: number;
export declare const PAIRING_RPC_OPTS: Record<PairingJsonRpcTypes.WcMethod | "unregistered_method", {
    req: RelayerTypes.PublishOptions;
    res: RelayerTypes.PublishOptions;
}>;
export declare const PAIRING_EVENTS: {
    create: string;
    expire: string;
    delete: string;
    ping: string;
};
//# sourceMappingURL=pairing.d.ts.map