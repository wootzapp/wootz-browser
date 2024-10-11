export type AccountPermissions = {
    approvedTargets: string[] | "*";
    nativeTokenLimitPerTransaction?: number | string;
    permissionStartTimestamp?: Date;
    permissionEndTimestamp?: Date;
    reqValidityStartTimestamp?: Date;
    reqValidityEndTimestamp?: Date;
};
export declare const SignerPermissionRequest: {
    name: string;
    type: string;
}[];
//# sourceMappingURL=types.d.ts.map