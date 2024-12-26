type PackageDirectoryOptions = {
    /**
     * The directory to start searching from.
     * @default process.cwd()
     */
    readonly cwd?: string;
};
export declare function packageDirectory({ cwd }?: PackageDirectoryOptions): Promise<string | undefined>;
export {};
//# sourceMappingURL=utils.d.ts.map