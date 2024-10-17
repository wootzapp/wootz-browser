import { Message, MessageID } from './Message';
import { SerializedEthereumRpcError } from '../error';
import { AppMetadata } from '../provider/interface';
interface RPCMessage extends Message {
    id: MessageID;
    sender: string;
    content: unknown;
    timestamp: Date;
}
export type EncryptedData = {
    iv: ArrayBuffer;
    cipherText: ArrayBuffer;
};
export interface RPCRequestMessage extends RPCMessage {
    content: {
        handshake: RequestAccountsAction;
    } | {
        encrypted: EncryptedData;
    };
}
export interface RPCResponseMessage extends RPCMessage {
    requestId: MessageID;
    content: {
        encrypted: EncryptedData;
    } | {
        failure: SerializedEthereumRpcError;
    };
}
type RequestAccountsAction = {
    method: 'eth_requestAccounts';
    params: AppMetadata;
};
export {};
