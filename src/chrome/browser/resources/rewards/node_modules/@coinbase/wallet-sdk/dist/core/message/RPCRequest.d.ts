import { RequestArguments } from '../provider/interface';
export type RPCRequest = {
    action: RequestArguments;
    chainId: number;
};
