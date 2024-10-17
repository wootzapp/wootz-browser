import { StateUpdateListener } from './interface';
import { Communicator } from '../core/communicator/Communicator';
import { SignerType } from '../core/message';
import { AppMetadata, Preference, Signer } from '../core/provider/interface';
export declare function loadSignerType(): SignerType | null;
export declare function storeSignerType(signerType: SignerType): void;
export declare function fetchSignerType(params: {
    communicator: Communicator;
    preference: Preference;
    metadata: AppMetadata;
}): Promise<SignerType>;
export declare function createSigner(params: {
    signerType: SignerType;
    metadata: AppMetadata;
    communicator: Communicator;
    updateListener: StateUpdateListener;
}): Signer;
