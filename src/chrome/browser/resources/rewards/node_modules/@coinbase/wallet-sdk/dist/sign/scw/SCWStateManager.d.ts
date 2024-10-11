import { StateUpdateListener } from '../interface';
import { AddressString, Chain } from '../../core/type';
export declare class SCWStateManager {
    private readonly storage;
    private readonly updateListener;
    private availableChains?;
    private _walletCapabilities?;
    private _accounts;
    private _activeChain;
    get accounts(): AddressString[];
    get activeChain(): Chain;
    get walletCapabilities(): Record<`0x${string}`, Record<string, unknown>> | undefined;
    constructor(params: {
        updateListener: StateUpdateListener;
        appChainIds: number[];
    });
    updateAccounts(accounts: AddressString[]): void;
    switchChain(chainId: number): boolean;
    updateAvailableChains(rawChains: {
        [key: number]: string;
    }): void;
    updateWalletCapabilities(capabilities: Record<`0x${string}`, Record<string, unknown>>): void;
    private storeItemToStorage;
    private loadItemFromStorage;
    clear(): void;
}
