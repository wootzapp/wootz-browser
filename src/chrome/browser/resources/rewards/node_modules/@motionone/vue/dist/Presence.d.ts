export declare const Presence: import("vue").DefineComponent<{
    name: {
        type: StringConstructor;
    };
    initial: {
        type: BooleanConstructor;
        default: boolean;
    };
    exitBeforeEnter: {
        type: BooleanConstructor;
        default: boolean;
    };
}, void, unknown, {}, {
    enter(element: Element): void;
    exit(element: Element, done: VoidFunction): void;
}, import("vue").ComponentOptionsMixin, import("vue").ComponentOptionsMixin, Record<string, any>, string, import("vue").VNodeProps & import("vue").AllowedComponentProps & import("vue").ComponentCustomProps, Readonly<import("vue").ExtractPropTypes<{
    name: {
        type: StringConstructor;
    };
    initial: {
        type: BooleanConstructor;
        default: boolean;
    };
    exitBeforeEnter: {
        type: BooleanConstructor;
        default: boolean;
    };
}>>, {
    initial: boolean;
    exitBeforeEnter: boolean;
}>;
//# sourceMappingURL=Presence.d.ts.map