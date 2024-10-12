import { IKeyValueStorage } from "../../shared";
export declare const migrate: (fromStore: string, toStore: IKeyValueStorage, onCompleteCallback: () => void) => Promise<void>;
export declare const beforeMigrate: (fromStore: string) => void;
//# sourceMappingURL=nodeMigration.d.ts.map