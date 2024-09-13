/// <reference types="node" />
import { Logger } from "@walletconnect/logger";
import { ICore, PairingTypes, IPairing, IStore } from "@walletconnect/types";
import EventEmitter from "events";
export declare class Pairing implements IPairing {
    core: ICore;
    logger: Logger;
    name: string;
    version: string;
    events: EventEmitter;
    pairings: IStore<string, PairingTypes.Struct>;
    private initialized;
    private storagePrefix;
    private ignoredPayloadTypes;
    private registeredMethods;
    constructor(core: ICore, logger: Logger);
    init: IPairing["init"];
    get context(): string;
    register: IPairing["register"];
    create: IPairing["create"];
    pair: IPairing["pair"];
    activate: IPairing["activate"];
    ping: IPairing["ping"];
    updateExpiry: IPairing["updateExpiry"];
    updateMetadata: IPairing["updateMetadata"];
    getPairings: IPairing["getPairings"];
    disconnect: IPairing["disconnect"];
    private sendRequest;
    private sendResult;
    private sendError;
    private deletePairing;
    private isInitialized;
    private cleanup;
    private registerRelayerEvents;
    private onRelayEventRequest;
    private onRelayEventResponse;
    private onPairingPingRequest;
    private onPairingPingResponse;
    private onPairingDeleteRequest;
    private onUnknownRpcMethodRequest;
    private onUnknownRpcMethodResponse;
    private registerExpirerEvents;
    private isValidPair;
    private isValidPing;
    private isValidDisconnect;
    private isValidPairingTopic;
}
//# sourceMappingURL=pairing.d.ts.map