import { IEvents } from "@walletconnect/events";
import { IHeartBeat } from "@walletconnect/heartbeat";
import { IKeyValueStorage, KeyValueStorageOptions } from "@walletconnect/keyvaluestorage";
import { ICrypto } from "./crypto";
import { IRelayer } from "./relayer";
import { IKeyChain } from "./keychain";
import { IJsonRpcHistory } from "./history";
import { IExpirer } from "./expirer";
import { IPairing } from "./pairing";
import { Logger } from "@walletconnect/logger";
import { IVerify } from "./verify";
import { IEchoClient } from "./echo";
import { IEventClient } from "./events";
export declare namespace CoreTypes {
    interface Options {
        projectId?: string;
        name?: string;
        relayUrl?: string;
        logger?: string | Logger;
        keychain?: IKeyChain;
        storage?: IKeyValueStorage;
        storageOptions?: KeyValueStorageOptions;
        maxLogBlobSizeInBytes?: number;
        customStoragePrefix?: string;
        telemetryEnabled?: boolean;
    }
    interface Metadata {
        name: string;
        description: string;
        url: string;
        icons: string[];
        verifyUrl?: string;
        redirect?: {
            native?: string;
            universal?: string;
            linkMode?: boolean;
        };
    }
}
export declare abstract class ICore extends IEvents {
    opts?: CoreTypes.Options | undefined;
    readonly protocol = "wc";
    readonly version = 2;
    abstract readonly name: string;
    abstract readonly context: string;
    abstract readonly relayUrl?: string;
    abstract readonly projectId?: string;
    abstract readonly customStoragePrefix: string;
    abstract logger: Logger;
    abstract heartbeat: IHeartBeat;
    abstract crypto: ICrypto;
    abstract relayer: IRelayer;
    abstract storage: IKeyValueStorage;
    abstract history: IJsonRpcHistory;
    abstract expirer: IExpirer;
    abstract pairing: IPairing;
    abstract verify: IVerify;
    abstract echoClient: IEchoClient;
    abstract linkModeSupportedApps: string[];
    abstract eventClient: IEventClient;
    constructor(opts?: CoreTypes.Options | undefined);
    abstract start(): Promise<void>;
    abstract dispatchEnvelope(params: {
        topic: string;
        message: string;
        sessionExists: boolean;
    }): void;
    abstract addLinkModeSupportedApp(universalLink: string): void;
}
//# sourceMappingURL=core.d.ts.map