import { ClientScopedStorage } from "../../in-app/core/authentication/client-scoped-storage.js";
import type { WalletConnectSession } from "./types.js";
/**
 * @internal
 */
export declare let walletConnectSessions: ClientScopedStorage | undefined;
/**
 * @internal
 */
export declare function getSessions(): Promise<WalletConnectSession[]>;
/**
 * @internal
 */
export declare function initializeSessionStore(options: {
    clientId: string;
}): void;
/**
 * @internal
 */
export declare function saveSession(session: WalletConnectSession): Promise<void>;
/**
 * @internal
 */
export declare function removeSession(session: WalletConnectSession): Promise<void>;
/**
 * @internal FOR TESTING ONLY
 */
export declare function setWalletConnectSessions(storage: ClientScopedStorage | undefined): void;
//# sourceMappingURL=session-store.d.ts.map