import { PropType, CSSProperties } from "vue";
import { VariantDefinition, InViewOptions, AnimationOptionsWithOverrides } from "@motionone/dom";
export declare const Motion: import("vue").DefineComponent<{
    tag: {
        type: StringConstructor;
        default: string;
    };
    initial: {
        type: PropType<boolean | VariantDefinition>;
    };
    animate: {
        type: PropType<VariantDefinition>;
    };
    inView: {
        type: PropType<VariantDefinition>;
    };
    hover: {
        type: PropType<VariantDefinition>;
    };
    press: {
        type: PropType<VariantDefinition>;
    };
    exit: {
        type: PropType<VariantDefinition>;
    };
    inViewOptions: {
        type: PropType<InViewOptions>;
    };
    transition: {
        type: PropType<AnimationOptionsWithOverrides>;
    };
    style: {
        type: PropType<CSSProperties>;
    };
}, {
    state: import("@motionone/dom").MotionState;
    root: import("vue").Ref<Element | null>;
    initialStyles: any;
}, unknown, {}, {}, import("vue").ComponentOptionsMixin, import("vue").ComponentOptionsMixin, Record<string, any>, string, import("vue").VNodeProps & import("vue").AllowedComponentProps & import("vue").ComponentCustomProps, Readonly<import("vue").ExtractPropTypes<{
    tag: {
        type: StringConstructor;
        default: string;
    };
    initial: {
        type: PropType<boolean | VariantDefinition>;
    };
    animate: {
        type: PropType<VariantDefinition>;
    };
    inView: {
        type: PropType<VariantDefinition>;
    };
    hover: {
        type: PropType<VariantDefinition>;
    };
    press: {
        type: PropType<VariantDefinition>;
    };
    exit: {
        type: PropType<VariantDefinition>;
    };
    inViewOptions: {
        type: PropType<InViewOptions>;
    };
    transition: {
        type: PropType<AnimationOptionsWithOverrides>;
    };
    style: {
        type: PropType<CSSProperties>;
    };
}>>, {
    tag: string;
}>;
//# sourceMappingURL=Motion.d.ts.map