import { Logger } from "@walletconnect/logger";
export declare namespace EchoClientTypes {
    type RegisterDeviceTokenParams = {
        clientId: string;
        token: string;
        notificationType: "fcm" | "apns" | "apns-sandbox" | "noop";
        enableEncrypted?: boolean;
    };
}
export declare abstract class IEchoClient {
    projectId: string;
    logger: Logger;
    abstract readonly context: string;
    constructor(projectId: string, logger: Logger);
    abstract registerDeviceToken(params: EchoClientTypes.RegisterDeviceTokenParams): Promise<void>;
}
//# sourceMappingURL=echo.d.ts.map