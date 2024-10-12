/**
 * @extension LENS
 */
export type LensProfileSchema = {
    signature: string;
    lens: {
        id: string;
        name: string;
        bio: string;
        picture: string;
        coverPicture: string;
        attributes: MetadataAttribute[];
        appId?: string;
    };
};
/**
 * @internal
 */
type MetadataAttribute = {
    type: "Boolean";
    key: string;
    value: "true" | "false";
} | {
    type: "Date";
    key: string;
    value: string;
} | {
    type: "Number";
    key: string;
    value: string;
} | {
    type: "String";
    key: string;
    value: string;
} | {
    type: "JSON";
    key: string;
    value: string;
};
export {};
//# sourceMappingURL=type.d.ts.map