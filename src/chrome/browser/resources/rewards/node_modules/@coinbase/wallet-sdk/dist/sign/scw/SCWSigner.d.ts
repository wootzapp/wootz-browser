import { StateUpdateListener } from '../interface';
import { Communicator } from '../../core/communicator/Communicator';
import { AppMetadata, RequestArguments, Signer } from '../../core/provider/interface';
import { AddressString } from '../../core/type';
export declare class SCWSigner implements Signer {
    private readonly metadata;
    private readonly communicator;
    private readonly keyManager;
    private readonly stateManager;
    constructor(params: {
        metadata: AppMetadata;
        communicator: Communicator;
        updateListener: StateUpdateListener;
    });
    handshake(): Promise<AddressString[]>;
    request<T>(request: RequestArguments): Promise<T>;
    disconnect(): Promise<void>;
    private tryLocalHandling;
    private sendEncryptedRequest;
    private createRequestMessage;
    private decryptResponseMessage;
    private updateInternalState;
}
