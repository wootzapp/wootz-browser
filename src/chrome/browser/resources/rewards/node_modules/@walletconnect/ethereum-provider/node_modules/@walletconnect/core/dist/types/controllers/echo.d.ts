import { Logger } from "@walletconnect/logger";
import { IEchoClient } from "@walletconnect/types";
export declare class EchoClient extends IEchoClient {
    projectId: string;
    logger: Logger;
    readonly context = "echo";
    constructor(projectId: string, logger: Logger);
    registerDeviceToken: IEchoClient["registerDeviceToken"];
}
//# sourceMappingURL=echo.d.ts.map